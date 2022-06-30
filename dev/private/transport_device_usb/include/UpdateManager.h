#pragma once

#include <unordered_set>
#include <vector>
#include <atomic>
// where is the encryption APIs?
//#include "cypher_tiny.h"
#include "FileStructure.h"
#include <thread>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <type_traits>

class FileSegment;
class UpdatePackage;

enum class UnpackStatusEnum
{
    NOT_STARTED,
    UNPACKING,
    SCRIPT_RUNNING,
    FAILED,
    SUCCESS
};

enum class UpdateFailures
{
    UNPACK_OK,
    UNPACK_FAILED,
    UNPACK_THREAD_START_FAILED,
    UNPACK_FILE_WRITE_FAILED,
    SHELL_SCRIPT_EXEC_FAILURE,
    SHELL_SCRIPT_GET_RESULT_TIMEOUT,
    SHELL_SCRIPT_RETURN_NON_ZERO,
    INCORRECT_SIGNATURE,
    INCORRECT_CFG_UPDATE,
    REMOVAL_KEYS_FAILURE,
};

class UpdateManager
{
public:
    UpdateManager(){
        InitializeStateMap();
    }
    ~UpdateManager();

    bool EraseAllFSStoredKeys();

    void SetPackedPackage(const unsigned char* buffer, const size_t& buffer_length);
    std::vector<EncFileSegment> GetUnpackedSegments();
    bool GetSignatureValidationResult();

    UnpackStatusEnum IsUpdateFinished();
    unsigned int GetErrorCode();
    UpdateFailures GetUpdateFailure() { return update_states_.load();}

    void FreeMemory(){
        session_key_.swap(session_key_);
        package_.FileSegments.swap(package_.FileSegments);
        package_.UpdatePackageBuffer.swap(package_.UpdatePackageBuffer);
    };
private:
    const int max_delay_between_two_outputs = 10;

    //bool CheckCanUpgradeFirmwareCfg(const std::string& old_cfg, const std::string& new_cfg);
    bool UpdateToConfig(const std::string& new_cfg);
    void BeginUnpackData();
    void UnpackWorker();
    bool RunUpdateScript(bool update_root, bool update_usr, bool update_var);
    void InitializeStateMap();
    std::string IntToHexStr(int n);
    void TrimEnd(std::string& str);
    template<typename T>
    typename std::enable_if<!std::is_integral<T>::value>::type HexStrToIntegral(std::string s)
    {
        static_assert(std::is_integral<T>::value, "Cannot instantiate to non-integral type!");
    }
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, T>::type HexStrToIntegral(std::string s)
    {
        T result;
        using namespace std;
        stringstream ss;
        ss << hex << s;
        ss >> result;
        return result;
    }


    std::vector<unsigned char> session_key_;
    EncUpdatePackage package_;
    std::atomic<UnpackStatusEnum> unpack_status_;
    std::atomic<UpdateFailures> update_states_;
    std::thread unpack_thread_;
    bool update_var_ = false;
    bool update_rootfs_ = false;
    bool update_usr_local_ = false;

    ManifestInfo manifest_;

    std::unordered_map<std::string, std::unordered_map<std::string, int>> state_map_;
};