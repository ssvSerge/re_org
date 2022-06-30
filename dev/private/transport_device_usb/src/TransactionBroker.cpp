#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <unordered_map>
#include "UpdateManager.h"
#include <sys/unistd.h>
#include <sys/reboot.h>
#include <dirent.h>
#include <sys/stat.h>
#include "TransactionBroker.h"
#include "V100Cmd.h"
#include "V100IDCmd.h"
#include "V100InternalCmd.h"
#include "logging.h"
#include "json.hpp"
#include "logging.h"

enum class LDSCommands : int {
    CMD_STOP_SENGINE          = 0x0010, // Shutdown the SEngine system
    CMD_ENABLE_LOGGING        = 0x0011, // Enable SELogger
    CMD_DISABLE_LOGGING       = 0x0012, // Disable SELogger
    CMD_UPDATE_LOGGING_CFG    = 0x0013, // Command SELogger to update its configuration from registry settings
    CMD_CONNECT_LOG_SVR       = 0x0014, // Command SELogger to connect to server pipe
    CMD_GET_CONFIG            = 0x0015, // Get sensor configuration
    CMD_SENGINE_SHUTTING_DOWN = 0x0016, // Warning that SENgine will be shut down shortly
    ACK                       = 0x0099, // Acknowledgement
};

struct ProcLive {
    std::string proc_name;
    bool proc_live;
};

typedef std::vector<ProcLive> LiveVec;

std::unordered_set<CommandItem, CommandItemHash, CommandItem::CommandItemComparer> TransactionBroker::handled_cmds_;
bool TransactionBroker::static_recovery_flag_ = false;
int32_t TransactionBroker::static_recovery_reason_ = 0;

extern std::string g_sensor_type_str;

bool TransactionBroker::ExecuteTransaction(ICmd* pCmd) {

    bool rc = false;
    // XXX - jbates - case statements from multiple (different) enums - switch on integral type instead
    uint uCmd = pCmd->GetCommandCode();

    info("ET: %s (cmd=%d)", cmd_to_string(uCmd).c_str(), uCmd);
    Atomic_Reset *pObject = nullptr;

    switch (uCmd) {
        case CMD_RESET:
            pObject = reinterpret_cast<Atomic_Reset *>(pCmd);
            pObject->SetReturnCode(GEN_OK);
            m_bShouldReboot = true;
            rc = true;
            break;
        case CMD_UPDATE_FIRMWARE:
            rc = UpdateFirmware(pCmd);
            break;
        case CMD_GET_OP_STATUS:
            rc = GetOpStatus(pCmd);
            break;
        case CMD_GET_CONFIG:
            rc = ExecuteGetConfig(pCmd);
            break;
        case CMD_GET_STATUS:
            rc = ExecuteGetStatus(pCmd);
            break;
        case CMD_GET_VERSION:
            rc = ExecuteGetVersion(pCmd);
            break;
        default:
            if (TransactionBroker::static_recovery_flag_)
            {
                pCmd->SetReturnCode(GEN_ERROR_DEVICE_UNCONFIGURED);
            }
            rc = false;
            break;
    }
    return rc;
}

bool TransactionBroker::Reset() {
    exit(0);
    return true;
}

bool TransactionBroker::Reset(ICmd *pCmd) {

    Atomic_Reset *pObject = reinterpret_cast<Atomic_Reset *>(pCmd);
    TransactionBroker::AddHandledCmd(CMD_GET_OP_STATUS, CmdFlagEnum::ATOMIC_FLAG | CmdFlagEnum::IGNORE_SE_FEEDBACK);
    pObject->SetReturnCode(GEN_OK);

    if ( !IsAfterUpdateReboot ) {
        USBCB header{};
        header.ulCount = sizeof(LDSCommands);

        if (shell_socket_client_.SendFrame((unsigned char*)&header, sizeof(USBCB), 0) == false) {
            err ("Error: Failed SendFrame", __FUNCTION__, __LINE__ );
            pObject->SetReturnCode(GEN_ERROR_INTERNAL);
            shell_socket_client_.StopClient();
            return false;
        }

        auto cmd = LDSCommands::CMD_STOP_SENGINE;
        if (shell_socket_client_.SendFrame((unsigned char*)&cmd, sizeof(LDSCommands), 0) == false) {
            err ("Error: Failed SendFrame", __FUNCTION__, __LINE__ );
            pObject->SetReturnCode(GEN_ERROR_INTERNAL);
            shell_socket_client_.StopClient();
            return false;
        }

        memset(&op_state_, 0, sizeof(_V100_OP_STATUS));
        op_state_.eMacroCommand = CMD_RESET;
        op_state_.eAcqStatus = ACQ_BUSY;
        op_state_.nMode = OP_IN_PROGRESS;
        unsigned char buffer[4]{};
        uint size = 4;
        if (shell_socket_client_.RecvFrame(buffer, size, 0) == false) {
            pObject->SetReturnCode(GEN_ERROR_INTERNAL);
            err ("Error: Failed RecvFrame", __FUNCTION__, __LINE__ );
            shell_socket_client_.StopClient();
        }

        if (!InitReboot) {
            return true;
        }
    }

    exit(0);
    return true;
}

bool TransactionBroker::UpdateFirmware(ICmd* pCmd) {

    Macro_Update_Firmware* pObject = reinterpret_cast<Macro_Update_Firmware*>(pCmd);

    if (!TransactionBroker::static_recovery_flag_) {
        ICmd* ResetCmd = new Atomic_Reset();
        InitReboot = false;
        Reset(ResetCmd); // Turn off SEngine first to free some memory.
        delete ResetCmd;
        ResetCmd = nullptr;
    }

    TransactionBroker::AddHandledCmd(CMD_GET_OP_STATUS, CmdFlagEnum::ATOMIC_FLAG | CmdFlagEnum::IGNORE_SE_FEEDBACK);
    auto data_size = pObject->GetDataSize();
    auto data_buffer = pObject->GetData();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if (data_size <= 0) {
        pObject->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }

    #define MAX_VCOM_MESSAGE_SIZE   1024*1024*100

    if (data_size > MAX_VCOM_MESSAGE_SIZE) {
        // Corrupt
        err("Firmware Size Corrupted in VCOM package");
        pObject->SetReturnCode(GEN_ERROR_PARAMETER);
        return false;
    }

    memset(&op_state_, 0, sizeof(_V100_OP_STATUS));
    op_state_.eMacroCommand = CMD_UPDATE_FIRMWARE;
    op_state_.eAcqStatus = ACQ_BUSY;
    op_state_.nMode = OP_IN_PROGRESS;

    update_manager_.SetPackedPackage(data_buffer, data_size);

    InitReboot = true;
    IsAfterUpdateReboot = true;
    return true;
}

bool TransactionBroker::GetOpStatus(ICmd* pCmd) {

    Atomic_Get_OP_Status* pObject = reinterpret_cast<Atomic_Get_OP_Status*>(pCmd);

    if (op_state_.eMacroCommand == CMD_RESET) {
        op_state_.eAcqStatus = ACQ_BUSY;
        op_state_.nMode = OP_IN_PROGRESS;
        pObject->SetReturnCode(GEN_OK);
        pObject->SetOPStatus(&op_state_);
        return true;
    }

    if (op_state_.eMacroCommand == CMD_UPDATE_FIRMWARE) {
        auto finished = update_manager_.IsUpdateFinished();
        if (finished == UnpackStatusEnum::SUCCESS) {
            auto fail_state = update_manager_.GetUpdateFailure();
            op_state_.eAcqStatus = ACQ_DONE;
            op_state_.nMode = (fail_state == UpdateFailures::UNPACK_OK ? OP_COMPLETE : OP_ERROR);
            op_state_.nParameter = (int)update_manager_.GetUpdateFailure();
        } else 
        if (finished == UnpackStatusEnum::FAILED) {
            op_state_.eAcqStatus = ACQ_ERROR_SYSTEM;
            op_state_.nMode = OP_ERROR;
            // Translate update failure to op error codes
            switch(update_manager_.GetUpdateFailure()) {
                case UpdateFailures::UNPACK_FAILED:
                    op_state_.nParameter = ERROR_UPDATE_UNPACK_FAILURE;
                    break;
                case UpdateFailures::UNPACK_THREAD_START_FAILED:
                    op_state_.nParameter = ERROR_UPDATE_THREAD_START;
                    break;
                case UpdateFailures::UNPACK_FILE_WRITE_FAILED:
                    op_state_.nParameter = ERROR_UPDATE_FLASH_WRITE_ERROR;
                    break;
                case UpdateFailures::SHELL_SCRIPT_EXEC_FAILURE:
                    op_state_.nParameter = ERROR_UPDATE_EXEC_SCRIPT_FAIL;
                    break;
                case UpdateFailures::SHELL_SCRIPT_GET_RESULT_TIMEOUT:
                    op_state_.nParameter = ERROR_UPDATE_EXEC_SCRIPT_TIMEOUT;
                    break;
                case UpdateFailures::SHELL_SCRIPT_RETURN_NON_ZERO:
                    op_state_.nParameter = ERROR_UPDATE_EXEC_NON_ZERO;
                    break;
                case UpdateFailures::INCORRECT_SIGNATURE:
                    op_state_.nParameter = ERROR_UPDATE_FIRMWARE_INVALID;
                    break;
                case UpdateFailures::INCORRECT_CFG_UPDATE:
                    op_state_.nParameter = ERROR_UPDATE_CFG_INVALID;
                    break;
                case UpdateFailures::REMOVAL_KEYS_FAILURE:
                    op_state_.nParameter = ERROR_UPDATE_REMOVE_KEYS_FAILED;
                    break;
                default:
                    op_state_.nParameter = (int)update_manager_.GetUpdateFailure();
                    break;
            }

        } else {
            op_state_.eAcqStatus = ACQ_BUSY;
            op_state_.nMode = OP_IN_PROGRESS;
            op_state_.nParameter = 0;
        }

        pObject->SetReturnCode(GEN_OK);
        pObject->SetOPStatus(&op_state_);
    }

    return true;
}

bool TransactionBroker::ExecuteGetConfig(ICmd* pCmd) {

    Atomic_Get_Config *pObject = reinterpret_cast<Atomic_Get_Config*>(pCmd);
    _V100_INTERFACE_CONFIGURATION_TYPE ICT_Temp = GetICT();
    pObject->SetConfiguration(&ICT_Temp);
    pObject->SetReturnCode(GEN_OK);
    return true;
}

static void read_json_file(const char* filename, nlohmann::ordered_json& contents) {

    std::ifstream file_in(filename); 

    if (file_in.is_open()) {
        std::stringstream buffer;
        buffer << file_in.rdbuf();
        std::string content = buffer.str();
        contents = nlohmann::ordered_json::parse(content.c_str());
    }
}

bool TransactionBroker::ExecuteGetVersion(ICmd* pCmd) {

    Atomic_Get_Version *pObject = reinterpret_cast<Atomic_Get_Version*>(pCmd);

    constexpr char VersionSystemPath[] = "/version.json";
    constexpr char VersionSoftwarePath[] = "/usr/local/version.json";
    constexpr char VersionDataPath[] = "/var/version.json";
    nlohmann::ordered_json system{};
    nlohmann::ordered_json software{};
    nlohmann::ordered_json data{};
    read_json_file(VersionSystemPath, system);
    read_json_file(VersionSoftwarePath, software);
    read_json_file(VersionDataPath, data);
    std::map<std::string, std::string> version_map;

    for (const auto& os : system["version-OS"].items()) {
        if (os.value().is_string()) {
            std::string key_name = "os-" + os.key();
            version_map[key_name] = os.value();
        }
    }

    for (const auto& os : system["host"].items()) {
        if (os.value().is_string()) {
            std::string key_name = "host-" + os.key();
            version_map[key_name] = os.value();
        }
    }

    for (const auto& sw : software["version-SW"].items()) {
        if (sw.value().is_string()) {
            std::string key_name = "sw-" + sw.key();
            version_map[key_name] = sw.value();
        }
    }

    std::string complete =
        version_map["os-major"] + "." +
        version_map["os-minor"] + "." +
        version_map["os-patch"] + "." +
        version_map["os-commit"] + "." +
        version_map["sw-commit"];
    std::string shortver =
        version_map["sw-commit"].c_str();

    nlohmann::ordered_json version {
        {
            "version", {
                {"complete", complete},
                {"short", shortver},
                {"full",
                    {
                        {"system", system},
                        {"software", software},
                        {"data", data}
                    }
                }
            }
        }
    };

    auto ver_str = version.dump(4);
    pObject->SetVersion(const_cast<char*>(ver_str.c_str()), ver_str.size()+1);
    pObject->SetReturnCode(GEN_OK);
    return true;
}

_V100_INTERFACE_CONFIGURATION_TYPE TransactionBroker::GetICT() {

    using namespace std;
    _V100_INTERFACE_CONFIGURATION_TYPE ICT_Temp {};
    ICT_Temp.Vendor_Id = 0x1fae;
    ICT_Temp.Product_Id = (unsigned short) stoi("0x" + g_sensor_type_str);
    ICT_Temp.FW_Flash_Available = 1;
    ICT_Temp.Phy_Interface_Available = 5;
    ICT_Temp.Device_Cfg_Type = (unsigned short) FIRMWARE_TM_ERROR;
    nlohmann::json json_sn, json_ver;
    string sn_str = ReadEntireFile("/etc/HID/SensorConfig.json");
    string ver_str = ReadEntireFile("/version.json");

    if (TransactionBroker::static_recovery_flag_) {
        ICT_Temp.Device_Cfg_Type = (unsigned short) FIRMWARE_TM_RECOVERY;
    }

    if (sn_str != "") {
        json_sn = nlohmann::json::parse(sn_str);
        auto sn_long = json_sn["Device_Serial_Number"].get<uint32_t>();
        ICT_Temp.Device_Serial_Number = (uint16_t) sn_long;
        ICT_Temp.Device_Serial_Number_Ex = (uint16_t) sn_long >> 16;
    }

    if (ver_str != "") {
        json_ver = nlohmann::json::parse(ver_str);
        auto sw_key = json_ver["version-SW"]["commit"].get<string>();
        stringstream ss;
        ss << sw_key;
        ss >> ICT_Temp.Firmware_Rev;
    }

    return ICT_Temp;
}

static int32_t CheckProcessesLiveness(std::unordered_map<std::string, bool> process_interest) {

    DIR* dir = {};
    struct dirent* ent;
    char buf[512] = {0,};

    long  pid = {};
    char pname[100] = {0,};
    char state = {};
    FILE *fp=NULL;

    if (!(dir = opendir("/proc"))) {
        err("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL ) {
        long lpid = atol(ent->d_name);
        if(lpid < 0)
            continue;
        snprintf(buf, sizeof(buf), "/proc/%ld/stat", lpid);
        fp = fopen(buf, "r");

        if (fp) {
            if ( (fscanf(fp, "%ld (%[^)]) %c", &pid, pname, &state)) != 3 ){
                err("fscanf failed \n");
                fclose(fp);
                closedir(dir);
                return -1;
            }
            for( auto& ele : process_interest){
                if(ele.first.compare(pname) == 0 ){
                    ele.second = true;
                }
            }
            fclose(fp);
        }
    }
    closedir(dir);
    return 0;
}

bool TransactionBroker::ExecuteGetStatus(ICmd* pCmd) {

    Atomic_Get_Status *pObject = reinterpret_cast<Atomic_Get_Status*>(pCmd);
    _V100_INTERFACE_STATUS_TYPE status_struct {};

    if (!TransactionBroker::static_recovery_flag_) {

        std::unordered_map<std::string, bool> vecs = {
            {"SEngineShell", false},
            {"HBSEApp", false},
            {"shm", false}
        };

        CheckProcessesLiveness(vecs);

        if (vecs["SEngineShell"] == false) {
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_FUTURE_2;
        }

        if (vecs["HBSEApp"] == false) {
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_FUTURE_3;
        }

        if (vecs["shm"] == false) {
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_FUTURE_4;
        }

        if (vecs["SEngineShell"] == true && vecs["HBSEApp"] == true && vecs["shm"] == true) {
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_FUTURE_5;
        }

        pObject->SetInterfaceStatusType(status_struct);
        pObject->SetReturnCode(GEN_OK);
        return true;
    }

    switch (TransactionBroker::static_recovery_reason_) {
        case 123:
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_VDK;
            break;
        default:
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_BSP;
            status_struct.Boot_Error |= STATUS_BOOT_ERROR_TAMPER ;
            break;
    }

    pObject->SetInterfaceStatusType(status_struct);
    pObject->SetReturnCode(GEN_OK);
    return true;
}

std::string TransactionBroker::ReadEntireFile(const std::string& file_path_str) {

    using namespace std;
    ifstream::sync_with_stdio(false);
    ifstream ifs(file_path_str, ios::in | ios::binary);
    if (!ifs)
        return "";
    string ret_str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    ifstream::sync_with_stdio(true);
    return ret_str;
}

int32_t TransactionBroker::Initialize() {

    TransactionBroker::AddHandledCmd(CMD_RESET, CmdFlagEnum::MACRO_FLAG | CmdFlagEnum::TRANSPORT_TO_SE | CmdFlagEnum::IGNORE_SE_FEEDBACK);
    TransactionBroker::AddHandledCmd(CMD_UPDATE_FIRMWARE, CmdFlagEnum::MACRO_FLAG | CmdFlagEnum::SHUTDOWN_SE | CmdFlagEnum::IGNORE_SE_FEEDBACK);

    char shell_socket_name_buffer[128]{};
    snprintf(shell_socket_name_buffer, 128, "SEShell_External_%03d_%03d", 0, 0);
    shell_socket_name = std::string(shell_socket_name_buffer);

    auto success = shell_socket_client_.InitSocket(shell_socket_name, true);
    if (!success)
    {
        err("ERROR: SEngine shell socket init failed!!!");
    }
    success = false;
    int fail_count = 0;
    while (!success)
    {
        success = shell_socket_client_.StartClient();
        if (!success)
        {
            err("ERROR: SEngine shell socket server cannot start!");
            fail_count++;
        }
        if (fail_count > 2)
        return 1;
    }
    return 0;
}

ICmd *TransactionBroker::GetCmdContainer(_V100_COMMAND_SET nC, const uint8_t *szPacket, uint bytesRx)
{
    _V100_COMMAND_SET nCmd = (nC == CMD_NONE) ? PeekCommand(szPacket, bytesRx) : nC;

    ICmd *pGenericCommand = NULL;

    // XXX - jbates - case statements from multiple (different) enums - switch on integral type instead
    unsigned uCmd = nCmd;

    switch (uCmd)
    {
    case CMD_RESET:
        pGenericCommand = new Atomic_Reset();
        break;
    case CMD_UPDATE_FIRMWARE:
        pGenericCommand = new Macro_Update_Firmware();
        break;
    case CMD_GET_OP_STATUS:
        pGenericCommand = new Atomic_Get_OP_Status();
        break;
    case CMD_GET_CONFIG:
        pGenericCommand = new Atomic_Get_Config();
        break;
    case CMD_GET_STATUS:
        pGenericCommand = new Atomic_Get_Status();
        break;
    case CMD_GET_VERSION:
        pGenericCommand = new Atomic_Get_Version();
        break;
    default:
    {
        Atomic_Error *pErr = new Atomic_Error();
        pErr->SetErrorCode(GEN_ERROR_APP_BUSY);
        pGenericCommand = reinterpret_cast<ICmd *>(pErr);
        return pGenericCommand;
    }
    break;
    }

    if (pGenericCommand)
    {
        if (!pGenericCommand->UnpackChallenge(szPacket, bytesRx))
        {
            delete pGenericCommand;
            pGenericCommand = NULL;
        }
    }
    return pGenericCommand;
}

_V100_COMMAND_SET TransactionBroker::PeekCommand(const uchar* szPacket, uint bytesRx)
{
    // Parse buffer to see whats what

    int16_t header = 0;
    _V100_COMMAND_SET command = CMD_NONE;
    memcpy(&header, szPacket, 2);
    if(header != SOHV)
    {
        return CMD_NONE;
    }
    memcpy(&command, &szPacket[2], 2);
    return command;
}

bool TransactionBroker::IsHandledCmd(const uchar* szPacket, uint bytesRx, CommandItem &cmd_found)
{
    _V100_COMMAND_SET target_cmd = PeekCommand(szPacket, bytesRx);
    auto cmd_target = TransactionBroker::handled_cmds_.find(target_cmd);

    if (cmd_target != TransactionBroker::handled_cmds_.end())
    {
        cmd_found = *cmd_target;
        return true;
    }
    else{
        if (static_recovery_flag_)
        {
            err("static recover flag = true! recovery mode!");
            cmd_found.Command = target_cmd;
            cmd_found.CmdFlags = CmdFlagEnum::IGNORE_SE_FEEDBACK | CmdFlagEnum::ATOMIC_FLAG;
            return true;
        }
    }
    cmd_found.Command = CMD_NONE;
    cmd_found.CmdFlags = CmdFlagEnum::UNHANDLED | CmdFlagEnum::TRANSPORT_TO_SE;
    return false;
}

void TransactionBroker::AddHandledCmd(_V100_COMMAND_SET target_cmd, int flags)
{
    handled_cmds_.insert(CommandItem(target_cmd, flags));
}

std::string TransactionBroker::cmd_to_string(uint cmd)
{
    #define SET_STR(A) \
    case A:{            \
        strCmd = #A;    \
    } break;            \

    std::string strCmd;
    switch(cmd){
        SET_STR(CMD_MATCH);
        SET_STR(CMD_VID_STREAM);
        SET_STR(CMD_GET_IMAGE);
        SET_STR(CMD_GET_COMPOSITE_IMAGE);
        SET_STR(CMD_SET_COMPOSITE_IMAGE);
        SET_STR(CMD_GET_FIR_IMAGE);
        SET_STR(CMD_GET_TEMPLATE);
        SET_STR(CMD_SET_TEMPLATE);
        SET_STR(CMD_ARM_TRIGGER);
        SET_STR(CMD_GET_ACQ_STATUS);
        SET_STR(CMD_GET_CONFIG);
        SET_STR(CMD_GET_STATUS);
        SET_STR(CMD_GET_CAL);
        SET_STR(CMD_GET_CMD);
        SET_STR(CMD_SET_CMD);
        SET_STR(CMD_SET_CAL);
        SET_STR(CMD_SET_RECORD);
        SET_STR(CMD_WRITE_FLASH);
        SET_STR(CMD_GET_RECORD);
        SET_STR(CMD_GET_SERIAL_NUMBER);
        SET_STR(CMD_SET_LED);
        SET_STR(CMD_SET_LICENSE_KEY);
        SET_STR(CMD_GET_LICENSE_KEY);
        SET_STR(CMD_GET_OP_STATUS);
        SET_STR(CMD_GET_DB_METRICS);
        SET_STR(CMD_ID_IDENTIFY);
        SET_STR(CMD_LOOPBACK);
        SET_STR(CMD_CONFIG_COMPORT);
        SET_STR(CMD_PROCESS);
        SET_STR(CMD_RESET);
        SET_STR(CMD_TEST);
        SET_STR(CMD_START_BURNIN);
        SET_STR(CMD_STOP_BURNIN);
        SET_STR(CMD_GET_SYSTEM_STATE);
        SET_STR(CMD_SET_OPTION);
        SET_STR(CMD_WRITE_FILE);
        SET_STR(CMD_READ_FILE);
        SET_STR(CMD_GET_SPOOF_DETAILS);
        SET_STR(CMD_GET_SPOOF_DETAILS_V2);
        SET_STR(CMD_MATCH_EX);
        SET_STR(CMD_SPOOF_GET_TEMPLATE);
        SET_STR(CMD_GET_TAG);
        SET_STR(CMD_SET_TAG);
        SET_STR(CMD_TRUNCATE_378);
        SET_STR(CMD_SET_EEPROM);
        SET_STR(CMD_GET_EEPROM);
        SET_STR(CMD_SET_DSM_EEPROM);
        SET_STR(CMD_GET_DSM_EEPROM);
        SET_STR(CMD_SET_EEPROM_M320);
        SET_STR(CMD_GET_EEPROM_M320);
        SET_STR(CMD_ID_CREATE_DB);
        SET_STR(CMD_ID_SET_WORKING_DB);
        SET_STR(CMD_ID_GET_USER_RECORD);
        SET_STR(CMD_ID_SET_USER_RECORD);
        SET_STR(CMD_ID_ENROLL_USER_RECORD);
        SET_STR(CMD_ID_IDENTIFY_378);
        SET_STR(CMD_ID_DELETE_DB);
        SET_STR(CMD_ID_GET_DB_METRICS);
        SET_STR(CMD_ID_GET_SYSTEM_METRICS);
        SET_STR(CMD_ID_GET_RESULT);
        SET_STR(CMD_ID_DELETE_USER_RECORD);
        SET_STR(CMD_ID_GET_USER_RECORD_HEADER);
        SET_STR(CMD_ID_GET_PARAMETERS);
        SET_STR(CMD_ID_SET_PARAMETERS);
        SET_STR(CMD_ID_VERIFY_USER_RECORD);
        SET_STR(CMD_ID_VERIFY_378);
        SET_STR(CMD_ID_VERIFY_MANY);
        SET_STR(CMD_ID_GET_TEMPLATE);
        SET_STR(CMD_ID_GET_IMAGE);
        SET_STR(CMD_ID_GET_SPOOF_SCORE);
        SET_STR(CMD_ID_SET_API_KEY);
        SET_STR(CMD_ID_COMMIT);
        SET_STR(CMD_ID_PURGE_DB_ALL);
        SET_STR(CMD_CANCEL_OPERATION);
        SET_STR(CMD_SET_GPIO);
        SET_STR(CMD_GET_GPIO);
        SET_STR(CMD_VERIFY_378);
        SET_STR(CMD_ERROR);
        SET_STR(CMD_UPDATE_FIRMWARE);
        SET_STR(CMD_GET_VERSION);
        SET_STR(CMD_ENC_CHALLENGE);
        SET_STR(CMD_ENC_GET_IMAGE);
        SET_STR(CMD_ENC_GET_NONCE);
        SET_STR(CMD_ENC_GET_TEMPLATE);
        SET_STR(CMD_ENC_LOCK_KEY);
        SET_STR(CMD_ENC_SET_ACTIVE_KEY);
        SET_STR(CMD_ENC_UNLOCK_KEY);
        SET_STR(CMD_ENC_VERIFY);
        SET_STR(CMD_ENC_GET_DIAG_STATUS);
        SET_STR(CMD_ENC_GET_PARAMETERS);
        SET_STR(CMD_ENC_MATCH);
        SET_STR(CMD_ENC_GET_SERIAL_NUMBER);
        SET_STR(CMD_ENC_GET_RND_NUMBER);
        SET_STR(CMD_ENC_DECRYPT);
        SET_STR(CMD_ENC_SET_PARAMETERS);
        SET_STR(CMD_ENC_VERIFYMATCH);
        SET_STR(CMD_ENC_ENROLL);
        SET_STR(CMD_ENC_RETURNCAPTUREDIMAGE);
        SET_STR(CMD_ENC_RETURNCAPTUREDBIR_IM);
        SET_STR(CMD_ENC_GET_CAPTURE_STATS);
        SET_STR(CMD_ENC_CAPTURE);
        SET_STR(CMD_ENC_RETURNCAPTUREDBIR);
        SET_STR(CMD_ENC_FACTORY_SET_KEY);
        SET_STR(CMD_ENC_VERIFYMATCH_RESULT);
        SET_STR(CMD_ENC_VERIFYMATCH_MANY);
        SET_STR(CMD_ENC_GENERATE_SESSIONKEY);
        SET_STR(CMD_ENC_GET_KEY);
        SET_STR(CMD_ENC_GET_KEYVERSION);
        SET_STR(CMD_ENC_SET_KEY);
        SET_STR(CMD_ENC_SET_ACTIVEKEY);
        SET_STR(CMD_ENC_GET_SPOOF_SCORE);
        SET_STR(CMD_ENC_RETURNCAPTUREDWSQ);
        SET_STR(CMD_ENC_CLEAR);
        SET_STR(CMD_ENC_GET_SENSOR_INFO);
        SET_STR(CMD_ENC_RETURNCAPTUREDTEMPLATE);
        SET_STR(CMD_ENC_GENERATE_RSA_KEYS);
        SET_STR(CMD_ENC_GET_KCV);
        SET_STR(CMD_ENC_SET_SESSION_KEY);
        SET_STR(CMD_ENC_IDENTIFYMATCH);
        SET_STR(CMD_HID_INIT);
        SET_STR(CMD_HID_ENUM_CAMS);
        SET_STR(CMD_HID_TERMINATE);
        SET_STR(CMD_HID_SET_PARAM_INT);
        SET_STR(CMD_HID_GET_PARAM_INT);
        SET_STR(CMD_HID_SET_PARAM_STR);
        SET_STR(CMD_HID_GET_PARAM_STR);
        SET_STR(CMD_HID_SET_PARAM_BIN);
        SET_STR(CMD_HID_GET_PARAM_BIN);
        SET_STR(CMD_HID_SET_PARAM_LONG);
        SET_STR(CMD_HID_GET_PARAM_LONG);
        SET_STR(CMD_HID_CAPTURE_IMAGE);
        SET_STR(CMD_HID_OPEN_CONTEXT);
        SET_STR(CMD_HID_CLOSE_CONTEXT);
        SET_STR(CMD_HID_STOP_CMD_ASYNC);
        SET_STR(CMD_HID_ASYNC_EXTRACT_TEMPLATE);
        SET_STR(CMD_HID_ASYNC_IDENTIFY_WITH_TEMPLATE);
        SET_STR(CMD_HID_ASYNC_MATCH_WITH_TEMPLATE);
        SET_STR(CMD_HID_ASYNC_MATCH_WITH_CAPTURED);
        SET_STR(CMD_HID_ASYNC_VERIFY_WITH_CAPTURED);
        SET_STR(CMD_HID_ASYNC_IDENTIFY_WITH_CAPTURED);
        SET_STR(CMD_HID_ASYNC_VERIFY_WITH_TEMPLATE);
        SET_STR(CMD_HID_GET_INTERMEDIATE_RES);
        SET_STR(CMD_HID_GET_FINAL_RES);
        SET_STR(CMD_HID_PARSE_RES_INT);
        SET_STR(CMD_HID_PARSE_RES_DOUBLE);
        SET_STR(CMD_HID_PARSE_RES_DATA);
        SET_STR(CMD_HID_PARSE_RES_POINT);
        SET_STR(CMD_HID_PARSE_RES_IMAGE);
        SET_STR(CMD_HID_PARSE_MATCH_GALLERY);
        SET_STR(CMD_HID_DB_ADD_RECORD_WITH_CAPTURED);
        SET_STR(CMD_HID_DB_ADD_RECORD_WITH_TEMPLATE);
        SET_STR(CMD_HID_DB_GET_RECORD);
        SET_STR(CMD_HID_DB_LIST_RECORDS);
        SET_STR(CMD_HID_DB_DEL_RECORD);
        SET_STR(CMD_HID_GET_VIDEO_FRAME);

        default:
            strCmd = "Unrecognized Command";
    };
    return strCmd;
}

void TransactionBroker::SetRecoveryMode(int32_t reason_num)
{
    err("TBroker: In Recovery Mode!");
    TransactionBroker::static_recovery_flag_ = true;
    TransactionBroker::static_recovery_reason_ = reason_num;
    TransactionBroker::AddHandledCmd(CMD_GET_CONFIG, CmdFlagEnum::ATOMIC_FLAG | CmdFlagEnum::IGNORE_SE_FEEDBACK);
    TransactionBroker::AddHandledCmd(CMD_GET_STATUS, CmdFlagEnum::ATOMIC_FLAG | CmdFlagEnum::IGNORE_SE_FEEDBACK);
    TransactionBroker::AddHandledCmd(CMD_GET_VERSION, CmdFlagEnum::ATOMIC_FLAG | CmdFlagEnum::IGNORE_SE_FEEDBACK);
    update_manager_.EraseAllFSStoredKeys();
}