#pragma once
#include "ISecureElement.h"
#include "SocketServer.h"

#include <atomic>
#include <thread>
#include <tuple>
#include <vector>

class HBSETransceiver
{
public:
    bool StartServer();
    void ExitApp();
    std::tuple<CryptExecStatus, std::vector<uint8_t>> HandleCommand(uint32_t cmd, const uint8_t *buffer,
                                                                    uint32_t buffer_length);
    void GetResponseBuffer(uint8_t **buffer, uint32_t &buffer_length);
    bool TA_CloseSession();
    bool TA_OpenSession();
    void SetDebugMode(bool start_test_mode);

private:
    static void InitHandlers();
    void NewConnectionHandler(int conn_file_descriptor);

    SocketServer m_server_instance;
    ISecureElement *m_executor;
    std::vector<std::thread> m_msg_threads;
    std::atomic<std::thread::id> m_last_thread_id;
    std::atomic<int> m_running;
    std::atomic<int> m_debug_mode {0};
};
