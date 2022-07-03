#include <thread>
#include <chrono>
#include "UpdateManager.h"
#include "FileStructure.h"
#include "RemoteExec.h"
#include <fstream>
#include <sys/types.h>
#include <functional>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <json.hpp>
#include <sstream>
#include <iomanip>
#include "FirmwareConfigParser.h"
#include "logging.h"
#include "ISecureElement.h"

extern std::string g_sensor_type_str;

// From Stackoverflow: https://stackoverflow.com/questions/53365538/how-to-determine-whether-to-use-filesystem-or-experimental-filesystem
#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

// Check for feature test macro for <filesystem>
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0

// Check for feature test macro for <experimental/filesystem>
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// We can't check if headers exist...
// Let's assume experimental to be safe
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Check if the header "<filesystem>" exists
#   elif __has_include(<filesystem>)

// If we're compiling on Visual Studio and are not compiling with C++17, we need to use experimental
#       ifdef _MSC_VER

// Check and include header that defines "_HAS_CXX17"
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>

// Check for enabled C++17 support
#               if defined(_HAS_CXX17) && _HAS_CXX17
// We're using C++17, so let's use the normal version
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif

// If the marco isn't defined yet, that means any of the other VS specific checks failed, so we need to use experimental
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif

// Not on Visual Studio. Let's use the normal version
#       else // #ifdef _MSC_VER
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif

// Check if the header "<filesystem>" exists
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

// Fail if neither header is available with a nice error message
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif

// We priously determined that we need the exprimental version
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
// Include it
#       include <experimental/filesystem>

// We need the alias from std::experimental::filesystem to std::filesystem
namespace std {
    namespace filesystem = experimental::filesystem;
}

// We have a decent compiler and can use the normal version
#   else
// Include it
#       include <filesystem>
#   endif

#endif // #ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

#define LOG_INFO(...) info(__VA_ARGS__)

bool UpdateManager::EraseAllFSStoredKeys()
{
    using namespace std;
    auto keys_iterator = filesystem::recursive_directory_iterator("/var/data/local/secure_keys");
    vector<filesystem::path> to_delete_files;
    bool is_all_success = true;
    for(const auto& it: keys_iterator)
    {
        if(filesystem::is_regular_file(it))
        {
            to_delete_files.emplace_back(filesystem::absolute(it.path()));
        }
    }
    auto props_iterator = filesystem::recursive_directory_iterator("/var/data/local/sec_data_stor");
    for(const auto& it:props_iterator)
    {
        if (filesystem::is_regular_file(it))
        {
            to_delete_files.emplace_back(filesystem::absolute(it.path()));
        }
    }
    auto logs_iterator = filesystem::recursive_directory_iterator("/var/volatile/");
    for(const auto &it: logs_iterator)
    {
        if (filesystem::is_regular_file(it))
        {
            to_delete_files.emplace_back(filesystem::absolute(it.path()));
        }
    }
    for (const auto& it : to_delete_files)
    {
        fstream overwritter(it, ios::in | ios::out | ios::binary);
        if (overwritter)
        {
            overwritter.seekg(0, ios::end);
            size_t len = overwritter.tellg();
            overwritter.seekg(0, ios::beg);
            overwritter.write(0, len);
            overwritter.flush();
            sync(); sync(); sync();
            overwritter.seekg(0, ios::beg);
            if (len < 1024)
            {
                char *buf = new char[len];
                memset(buf, 0xff, len);
                overwritter.write(buf, len);
                overwritter.flush();
                sync();sync();sync();
                delete[] buf;
                buf = nullptr;
            }else{
                auto blocks = len / 1024;
                auto remainder = len % 1024;
                char* block_buf = new char[1024];
                memset(block_buf, 0xff, 1024);
                for(size_t index = 0; index < blocks; index++)
                {
                    overwritter.write(block_buf, 1024);
                }
                overwritter.write(block_buf, remainder);
                overwritter.flush();
                sync(); sync(); sync();
                delete[] block_buf;
                block_buf = nullptr;
            }
        }else{
            err("!!! Cannot overwrite the key %s !!!", it.string().c_str());
            is_all_success = false;
        }
        if (!filesystem::remove(it))
        {
            err("!!! Cannot remove file: %s !!!", it.string().c_str());
            is_all_success = false;
        }
    }
    return is_all_success;
}

void UpdateManager::SetPackedPackage(const unsigned char* buffer, const size_t& buffer_length)
{
    unpack_status_ = UnpackStatusEnum::NOT_STARTED;
    std::vector<unsigned char> vect_buffer(buffer, buffer+buffer_length);
    package_.UpdatePackageBuffer.swap(package_.UpdatePackageBuffer);
    package_.UpdatePackageBuffer.shrink_to_fit();
    package_.UpdatePackageBuffer = std::move(vect_buffer);
    BeginUnpackData();
    return;
}

void UpdateManager::BeginUnpackData()
{
    unpack_status_ = UnpackStatusEnum::UNPACKING;
    unpack_thread_ = std::thread(&UpdateManager::UnpackWorker, this);
    if (!unpack_thread_.joinable())
    {
        update_states_ = UpdateFailures::UNPACK_THREAD_START_FAILED;
    }
}

UpdateManager::~UpdateManager()
{
    if (unpack_thread_.joinable())
    {
        unpack_thread_.join();
    }
}

void UpdateManager::UnpackWorker()
{
    using namespace std;
    this_thread::sleep_for(chrono::milliseconds(100));
    bool unpack_successful = false;
    try {
        unpack_successful = package_.UnpackUpdatePayload();
    }
    catch (...)
    {
        err("Unpack update payload failed! From unpack...");
        unpack_successful = false;
        update_states_ = UpdateFailures::UNPACK_FAILED;
        unpack_status_ = UnpackStatusEnum::FAILED;
        return;
    }
    if (!unpack_successful)
    {
        update_states_ = UpdateFailures::INCORRECT_SIGNATURE;
        unpack_status_ = UnpackStatusEnum::FAILED;
        return;
    }

    const string path_prefix = "/var/system/";
#if _WIN32
    _mkdir(path_prefix.c_str());
#else
    mode_t mode = 733;
    mkdir(path_prefix.c_str(), mode);
#endif
    bool UpdatedSysImages = false;
    bool UpdateConfigFiles = false;
    string NewConfigItem = "";
    vector<string> written_files;
    nlohmann::json jobj;
    for (auto file_seg : package_.FileSegments)
    {
        fstream bin_operator;
        std::string file_name_str;

        switch (file_seg.GetFileOperation())
        {
        case FileOperationTypes::SYSTEM_IMAGE:
        {
            debug("writing to /var/system/%s\n", file_seg.FileName());
            file_name_str = file_seg.FileNameStr();
            auto upd_var_str_find = file_name_str.find("system-img-var", 0);
            auto upd_root_str_find = file_name_str.find("system-img-rootfs", 0);
            auto upd_usr_str_find = file_name_str.find("system-img-usr-local", 0);
            debug("%d %d %d", upd_var_str_find, upd_root_str_find, upd_usr_str_find);
            if (upd_var_str_find != string::npos)
            {
                info("system-img-var\n");
                update_var_ = true;
            }
            if (upd_root_str_find != string::npos)
            {
                info("system-img-rootfs\n");
                update_rootfs_ = true;
            }
            if (upd_usr_str_find != string::npos)
            {
                info("system-img-usr-local\n");
                update_usr_local_ = true;
            }
            bin_operator.open((path_prefix + file_seg.FileNameStr()).c_str(), ios::out | ios::binary);
            bin_operator.write((const char*)file_seg.FileBuffer.data(), file_seg.FileBuffer.size());
            bin_operator.flush();
            UpdatedSysImages = true;
            if (bin_operator.bad())
            {
                update_states_ = UpdateFailures::UNPACK_FILE_WRITE_FAILED;
                unpack_status_ = UnpackStatusEnum::FAILED;
                return;
            }
            bin_operator.close();
            if (!manifest_.IsEmptyManifest())
            {
                manifest_.SetFileExistance(file_seg.FileName());
            }
            written_files.push_back(file_seg.FileNameStr());
        }
        break;
        case FileOperationTypes::MANIFEST:
        {
            info("Manifest file received. This update may have multiple segments!\n");
            jobj = nlohmann::json::parse(file_seg.FileBuffer.data());
            manifest_.Deserialize(jobj);
        }
        break;
        case FileOperationTypes::CONFIGURATION:
        {
            UpdateConfigFiles = true;
            NewConfigItem = string((char*)file_seg.FileBuffer.data());
            TrimEnd(NewConfigItem);
            info("Received a configuration file!");
        }
        break;
        case FileOperationTypes::DEBUG_SINGLE_FILE:
            bin_operator.open(file_seg.FileNameStr(), ios::out);
            bin_operator.write((char*)file_seg.FileBuffer.data(), file_seg.FileBuffer.size());
            bin_operator.flush();
            bin_operator.close();
        case FileOperationTypes::TEMP:
            // No longer accept this operation. No operation.
            break;
        case FileOperationTypes::RESTORE_SECURITY:
            ISecureElement::GetInstance()->Execute_Reset_Secure_Element();
            break;
        default:
            break;
        }
    }
    if (UpdateConfigFiles)
    {
        auto update_to_cfg = UpdateToConfig(NewConfigItem);
        if (!update_to_cfg)
        {
            // Failed, return immediately.
            return;
        }
    }
    sync();
    sync();
    sync();
    package_.FileSegments.clear();
    package_.FileSegments.shrink_to_fit();

    if (!manifest_.IsEmptyManifest())
    {
        info("Non-empty manifest file!");
        bool AllFilesExist = true;
        for (const auto& it : manifest_.GetExistanceMap())
        {
            const auto ExistNum = it.second;
            if ((ExistNum & 0b10) == 0)
            {
                AllFilesExist = false;
                break;
            }
        }
        if (AllFilesExist)
        {
            info("Non-empty manifest - All files exist!");
            auto script_result = RunUpdateScript(update_rootfs_, update_usr_local_, update_var_);

            if (!script_result)
            {
                if (update_states_ == UpdateFailures::UNPACK_OK)
                {
                    update_states_ = UpdateFailures::SHELL_SCRIPT_EXEC_FAILURE;
                }
            }
            else {
                update_states_ = UpdateFailures::UNPACK_OK;
            }
            unpack_status_ = UnpackStatusEnum::SUCCESS;
            return;
        }
        else {
            update_states_ = UpdateFailures::UNPACK_OK;
            unpack_status_ = UnpackStatusEnum::SUCCESS;
            return;
        }
    }
    else {
        if (UpdatedSysImages)
        {
            info("UpdateManager: Updated Sys Images");
            auto script_result = RunUpdateScript(update_rootfs_, update_usr_local_, update_var_);
            this_thread::sleep_for(chrono::seconds(1));
            info("Script result = %d", (int)script_result);
            if (!script_result)
            {
                if (update_states_ == UpdateFailures::UNPACK_OK)
                {
                    update_states_ = UpdateFailures::SHELL_SCRIPT_EXEC_FAILURE;
                }
                debug("Exiting UpdateSysImage with Script result fail !!");
                unpack_status_ = UnpackStatusEnum::SUCCESS;
                return;
            }
        }
        debug("Exiting UpdateSysImg with OK!");
        update_states_ = UpdateFailures::UNPACK_OK;
        unpack_status_ = UnpackStatusEnum::SUCCESS;
        return;
    }
}

bool UpdateManager::UpdateToConfig(const std::string& new_cfg)
{
    using namespace std;
    fstream bin_operator;
    bin_operator.open("/etc/HID/FirmwareConfig.txt", ios::in | ios::binary);
    string old_cfg((istreambuf_iterator<char>(bin_operator)), istreambuf_iterator<char>());
    TrimEnd(old_cfg);
    bin_operator.close();
    FirmwareConfigParser parser;
    if (0 != parser.LoadData())
    {
       warn("WARNING: Failed to load Firmware Config json data from file... Using default values");
    }
    string new_cfg_rw_str = "0x" + new_cfg;
    auto new_cfg_int = HexStrToIntegral<uint>(new_cfg_rw_str);
    auto old_cfg_int = HexStrToIntegral<uint>("0x"+old_cfg);

    if (new_cfg_int == old_cfg_int)
    {
        // Don't do anything (per documetation, same config update doesn't do anything.)
        return true;
    }
    string json_sensor_type_str = "";
    if (g_sensor_type_str == "0520")
    {
        json_sensor_type_str = "VENUS_V52X";
    }
    if (g_sensor_type_str == "0320")
    {
        json_sensor_type_str = "VENUS_V32X";
    }

    auto sensor_id = parser.GetSensorEnumByName(json_sensor_type_str.c_str());
    auto fw_update_state = parser.IsFwConfAllowed(sensor_id, old_cfg_int, new_cfg_int);

    switch (fw_update_state)
    {
        case FwUpdateStates::kNotAllowed:
        {
            // Document: Clear everything and not allow further update, and maybe signal sensor to reboot.
            for (auto dir_it : filesystem::directory_iterator("/var/system/"))
            {
                filesystem::remove(dir_it); // Clean-up
            }
            manifest_.ClearManifest();
            FreeMemory();

            update_states_ = UpdateFailures::INCORRECT_CFG_UPDATE;
            unpack_status_ = UnpackStatusEnum::FAILED;
            err("\nCannot update to this config!");
            return false;}
        case FwUpdateStates::kToBase:
        {
            // Remove all existing configuration keys from current slot.
            auto remove_keys = parser.GetFwConfKeySlots(old_cfg_int);
            auto remove_props = parser.GetFwConfProperties(old_cfg_int);
            for (auto key_item : remove_keys)
            {
                auto remove_success = ISecureElement::GetInstance()->Execute_Erase_Key(key_item.SecElemSlot);
                if (remove_success != CryptExecStatus::Successful)
                {
                    stringstream errstrm;
                    errstrm << "FwCfg " << hex << new_cfg_int << dec << " to base: configuration key " << key_item.EncKeyType << " - " << key_item.SecElemSlot << " removal failed, error: " << (int)remove_success << endl;
                    errstrm << "Will Continue." << endl;
                    warn(errstrm.str().c_str());
                }
            }
            for (auto prop_item : remove_props)
            {
                auto remove_success = ISecureElement::GetInstance()->Execute_Remove_Property((const uint8_t*)prop_item.c_str(), prop_item.length());
                if (remove_success != CryptExecStatus::Successful)
                {
                    stringstream errstrm;
                    errstrm << "FwCfg " << hex << new_cfg_int << dec << " to base: property " << prop_item << " removal failed, error: " << (int)remove_success << endl;
                    errstrm << "Will Continue." << endl;
                    warn(errstrm.str().c_str());
                }
            }
            // Write new config file.
            bin_operator.open("/etc/HID/FirmwareConfig.txt", ios::out | ios::trunc | ios::binary);
            bin_operator.flush();
            bin_operator << hex << (int)new_cfg_int << endl;
            bin_operator.flush();
            bin_operator.close();
            update_states_ = UpdateFailures::UNPACK_OK;
            unpack_status_ = UnpackStatusEnum::SUCCESS;
            return true;
        }
        case FwUpdateStates::kToNonBase:
        {
            // Write new config file.
            bin_operator.open("/etc/HID/FirmwareConfig.txt", ios::out | ios::trunc | ios::binary);
            bin_operator.flush();
            bin_operator << hex << (int)new_cfg_int << endl;
            bin_operator.flush();
            bin_operator.close();
            update_states_ = UpdateFailures::UNPACK_OK;
            unpack_status_ = UnpackStatusEnum::SUCCESS;
            return true;
        }
    }
    // Should never happen.
    err("\n!!!! UpdateManager Executed Below where it should be !!!\n");
    err("\n!!!! CHECK AT %s:%d\n", __FILE__, __LINE__ );
    return false;
}

bool UpdateManager::RunUpdateScript(bool update_root, bool update_usr, bool update_var)
{
    using namespace std;
    RemoteExec exec;

    string Opts = "";
    if ((!update_root) && (!update_usr) && (!update_var))
    {
        return true;
    }
    if (update_root && update_usr && update_var)
    {
        Opts = "A";
    }
    else {
        if (update_var)
        {
            Opts += "V";
        }
        if (update_usr)
        {
            Opts += "U";
        }
        if (update_root)
        {
            Opts += "R";
        }
    }
    info("Update Args = %s", ("-O0" + Opts).c_str());

    vector<string> exec_args{ "/usr/local/bin/system-update.sh", ("-O0" + Opts) };
    exec.set_exec_callback(function<RemoteExecStatusCallbackPrototype>() = [](const string& str)->void {
        info("EXEC: %s", str.c_str());
        });
    exec.execute_shell_command(exec_args, 60);
    auto status = exec.get_status();
    while (status == 0)
    {
        this_thread::sleep_for(chrono::seconds(1));
        status = exec.get_status();
        info("UpdateManager: RunUpdateScript: last status = %d", status);
    }
    debug("UpdateManager: exec_get_status is not 0, value = %d (Not an error)", status);
    auto ret_code = exec.get_return_code();
    if (ret_code != 0)
    {
        err("\nUpdate Exec Result = ", exec.get_status());
        err("\nUpdate Failed, return code is non-zero! code = %d", ret_code);
        update_states_ = UpdateFailures::SHELL_SCRIPT_RETURN_NON_ZERO;
        return false;
    }
    debug("UpdateManager: RunUpdateScript: returning!");
    return true;
}

unsigned int UpdateManager::GetErrorCode()
{
    return static_cast<unsigned int>(update_states_.load());
}

std::vector<EncFileSegment> UpdateManager::GetUnpackedSegments()
{
    if (unpack_status_.load() != UnpackStatusEnum::SUCCESS)
    {
        std::vector<EncFileSegment> empty_vect;
        return empty_vect;
    }
    return package_.FileSegments;
}
bool UpdateManager::GetSignatureValidationResult()
{
    if (unpack_status_.load() != UnpackStatusEnum::SUCCESS)
    {
        return false;
    }
    return package_.IsSignatureCorrect;
}
UnpackStatusEnum UpdateManager::IsUpdateFinished()
{
    return unpack_status_;
}

void UpdateManager::InitializeStateMap()
{
    using namespace std;
    for (int loop1 = 0x100;loop1 < 0x1500;loop1 += 256)
    {
        // Initialize all to not allowed.
        for (int loop2 = 0x100;loop2 < 0x1500; loop2 += 256)
        {
            if (loop2 >= 0xa00 && loop2 <= 0xf00)
                continue;
            state_map_[IntToHexStr(loop1)][IntToHexStr(loop2)] = 0;
        }
        if (loop1 >= 0xa00 && loop1 <= 0xf00)
            continue;

        // Allow convert to base
        state_map_[IntToHexStr(loop1)]["400"] = 1;
        // Allow base to this convert
        state_map_["400"][IntToHexStr(loop1)] = 1;
    }
}

std::string UpdateManager::IntToHexStr(int n)
{
    using namespace std;
    stringstream strm;
    strm << hex << n;
    return strm.str();
}

void UpdateManager::TrimEnd(std::string& str)
{
    using namespace std;
    int LastNonChrIndex = str.length();
    for(int i=str.length()-1; i>=0;i--)
    {
        if (str[i] == ' ' || str[i] == '\r' || str[i] == '\n')
        {
            LastNonChrIndex = i;
        }else{
            break;
        }
    }
    str.erase(LastNonChrIndex);
}