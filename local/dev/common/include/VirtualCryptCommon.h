#pragma once

#include <stdint.h>
#include <unordered_map>
#include <string>

#undef KCV_SIZE
constexpr unsigned int KCV_SIZE = 4;
enum class CryptoCommands : uint32_t
{
    NONE_CMD,
    GET_RAND,
    GET_RAND_INT,
    SELECT_KEY,
    ENCRYPT_DATA,
    DECRYPT_DATA,
    VERIFY_AND_DECRYPT_DATA,
    HASH_DATA,
    GENERATE_RSA_KEY,
    GENERATE_PLAT_KEYS,
    GET_KEY,
    GET_KEYINFO,
    SET_KEY,
    ERASE_KEY,
    SET_PROPERTY,
    GET_PROPERTY,
    ERASE_PROPERTY,
    GET_STATUS,
    CHECK_KEY,
    SELECT_DUKPT_KEY,
    SELECT_PUBLIC_RSA_KEY,

    _TEST,
    _DEBUG_MODE,
};
enum class CryptExecStatus : uint32_t
{
    Successful,
    Execute_Error,
    Encrypt_Error,
    Decrypt_Error,
    Hash_Error,
    Mem_Error,
    Invalid_Command,
    Not_Exist,
    Invalid_Argument,
    Not_Supported,
    Tamper,
    Cmd_Rejected,
    Not_Configured,
    Comm_Error,
    No_More_Sessions,
    Validation_Error,
    Execute_Exception, // the same as Execute_Error, but out params should be ignored
    In_Progress,
};
const std::unordered_map<CryptExecStatus, std::string> kExecStatusMapping = {
    {CryptExecStatus::Successful,"Successful"},
    {CryptExecStatus::Execute_Error,"Execute_Error"},
    {CryptExecStatus::Encrypt_Error,"Encrypt_Error"},
    {CryptExecStatus::Decrypt_Error,"Decrypt_Error"},
    {CryptExecStatus::Hash_Error,"Hash_Error"},
    {CryptExecStatus::Mem_Error,"Mem_Error"},
    {CryptExecStatus::Invalid_Command,"Invalid_Command"},
    {CryptExecStatus::Not_Exist,"Not_Exist"},
    {CryptExecStatus::Invalid_Argument,"Invalid_Argument"},
    {CryptExecStatus::Not_Supported,"Not_Supported"},
    {CryptExecStatus::Tamper,"Tamper"},
    {CryptExecStatus::Not_Configured,"Not_Configured"},
    {CryptExecStatus::Comm_Error,"Comm_Error"},
    {CryptExecStatus::No_More_Sessions,"No_More_Sessions"},
    {CryptExecStatus::Validation_Error,"Validation_Error"}
};

enum class AsyncExecStates
{
    In_Progress = 0,
    Exec_OK,
    Exec_Fail
};

struct AsyncExecResult
{
    AsyncExecStates async_result;
    CryptExecStatus exec_result;
};