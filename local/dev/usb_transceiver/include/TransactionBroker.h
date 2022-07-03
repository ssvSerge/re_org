#pragma once
#include "V100_shared_types.h"
#include "V100_internal_types.h"
#include <string>
#include <fstream>
#include "SocketClient.h"
#include "HandledCmdSet.h"
#include <unordered_set>
#include "UpdateManager.h"

class ICmd;

class BSPTester;



class TransactionBroker
{
public:
    // Class Factory
    TransactionBroker() = default;
    ~TransactionBroker() = default;
    // Init
    int32_t Initialize();
    // Execute the transaction
    bool ExecuteTransaction(ICmd *pCmd);
    static ICmd *GetCmdContainer(_V100_COMMAND_SET nC, const uint8_t *szPacket, uint bytesRx);
    bool Reset(ICmd* pCmd);
    bool Reset();
    bool Should_Reboot() { return m_bShouldReboot; }
    bool UpdateFirmware(ICmd* pCmd);
    bool GetOpStatus(ICmd* pCmd);
    // Peek Command
    static _V100_COMMAND_SET PeekCommand(const uchar *szPacket, uint bytesRx);
    static std::string cmd_to_string(uint cmd);
    // Check if command is handled by here.
    static bool IsHandledCmd(const uchar* szPacket, uint bytesRx, CommandItem &cmd_found);
    void SetRecoveryMode(int32_t reason_num);

private:
    static void AddHandledCmd(_V100_COMMAND_SET target_cmd, int flags);
    bool ExecuteGetConfig(ICmd* pCmd);
    bool ExecuteGetVersion(ICmd* pCmd);
    bool ExecuteGetStatus(ICmd* pCmd);

    std::string ReadEntireFile(const std::string& file_path_str);
    _V100_INTERFACE_CONFIGURATION_TYPE GetICT();
    std::string GetVersionStr();

    SocketClient shell_socket_client_;
    SocketClient shm_socket_client_;
    std::string shell_socket_name = "";
    std::string shm_socket_name = "shm";
    static std::unordered_set<CommandItem, CommandItemHash, CommandItem::CommandItemComparer> handled_cmds_;
    UpdateManager update_manager_;
    _V100_OP_STATUS op_state_ {};
    static bool static_recovery_flag_;
    static int32_t static_recovery_reason_;
    bool InitReboot = true;
    bool IsAfterUpdateReboot = false;
    bool m_bShouldReboot = false;

};
