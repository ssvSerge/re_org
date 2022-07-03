#include "HBSETransceiver.h"
#include <atomic>
#include <cassert>
#include <thread>

#include <packunpack.h>
#include <application/vcom_tools.h>

#include <VirtualSecureElement.h>

#include <logging.h>

// #define info     printf
// #define err      printf


using namespace packunpack::server_side;
using mydispatcher = dispatcher<CryptoCommands, ISecureElement *, CryptExecStatus, CryptExecStatus::Invalid_Command, CryptExecStatus::Execute_Error, CryptExecStatus::Execute_Exception>;


bool HBSETransceiver::StartServer() {
    static std::once_flag once;
    std::call_once(once, [this]() { InitHandlers(); });

    m_running = true;
    auto init_socket_successful = m_server_instance.InitSocket("TACmdSocketServer", true, true, false);
    if (!init_socket_successful) {
        fprintf(stdout, "Error: initialize socket server failed!\n");
        return false;
    }
    std::function<MsgHandleFunctionPrototype> func = std::bind(&HBSETransceiver::NewConnectionHandler, this, std::placeholders::_1);
    m_server_instance.BindConnectionHandleFunction(std::move(func));
    m_server_instance.StartServer();
    return true;
}

void HBSETransceiver::InitHandlers() {

    mydispatcher::add_handler<buffer_out<>>(CryptoCommands::GET_RAND, &ISecureElement::Execute_GetRandomBuffer);
    mydispatcher::add_handler<pod_in<uint32_t>>(CryptoCommands::SELECT_KEY, &ISecureElement::Execute_Select_Key);
    mydispatcher::add_handler<buffer_in<>, allocated_buffer_out<>, buffer_in<>>(CryptoCommands::ENCRYPT_DATA, &ISecureElement::Execute_Encrypt);
    mydispatcher::add_handler<buffer_in<>, allocated_buffer_out<>, buffer_in<>>(CryptoCommands::DECRYPT_DATA, &ISecureElement::Execute_Decrypt);
    mydispatcher::add_handler<buffer_in<>, buffer_in<>>( CryptoCommands::VERIFY_AND_DECRYPT_DATA, &ISecureElement::Execute_Verify_And_Decrypt);
    mydispatcher::add_handler<buffer_in<>, pod_in<HashAlgorithms>, allocated_buffer_out<>>( CryptoCommands::HASH_DATA, &ISecureElement::Execute_Hash_Data);
    mydispatcher::add_handler<>(CryptoCommands::GENERATE_RSA_KEY, &ISecureElement::Execute_Generate_RSA_Key);
    mydispatcher::add_handler<>(CryptoCommands::GENERATE_PLAT_KEYS, &ISecureElement::Execute_Reset_Secure_Element);
    mydispatcher::add_handler<allocated_buffer_out<>>(CryptoCommands::GET_KEY, &ISecureElement::Execute_Get_RSA_PublicKey);
    mydispatcher::add_handler<pod_in<uint32_t>, pod_out<KeyInfoStructure>>(CryptoCommands::GET_KEYINFO, &ISecureElement::Execute_Get_Key_Info);
    mydispatcher::add_handler<pod_in<KeyInfoStructure>, buffer_in<>>( CryptoCommands::SET_KEY, static_cast<CryptExecStatus (ISecureElement::*)(const KeyInfoStructure &, const uint8_t *, uint32_t)>(&ISecureElement::Execute_Set_Key));
    mydispatcher::add_handler<pod_in<uint32_t>>(CryptoCommands::ERASE_KEY, &ISecureElement::Execute_Erase_Key);
    mydispatcher::add_handler<>(CryptoCommands::GET_STATUS, &ISecureElement::Execute_Get_Status);
    mydispatcher::add_handler<buffer_in<>, buffer_in<>>(CryptoCommands::SET_PROPERTY, &ISecureElement::Execute_Set_Property);
    mydispatcher::add_handler<buffer_in<>, allocated_buffer_out<>>(CryptoCommands::GET_PROPERTY, &ISecureElement::Execute_Get_Property);
    mydispatcher::add_handler<buffer_in<>>(CryptoCommands::ERASE_PROPERTY, &ISecureElement::Execute_Remove_Property);
    mydispatcher::add_handler<pod_in<uint32_t>>(CryptoCommands::CHECK_KEY, &ISecureElement::Execute_Check_Key_Exist);
    mydispatcher::add_handler<pod_in<DUKPTKeySlots>>(CryptoCommands::SELECT_DUKPT_KEY, &ISecureElement::Execute_Select_DUKPT_Keys);
    mydispatcher::add_handler<pod_in<PublicRSAKeySlots>>(CryptoCommands::SELECT_PUBLIC_RSA_KEY, &ISecureElement::Execute_Select_Public_RSA_Keys);
    mydispatcher::add_handler<pod_in<uint32_t>, pod_in<double>, pod_in<std::pair<uint32_t, int32_t>>, pod_out<uint32_t>, buffer_in<>, buffer_out<>, allocated_buffer_out<>, buffer_in<uint32_t>>( CryptoCommands::_TEST, &ISecureElement::Execute__test);

    #if ENABLE_DEBUG_MODE
        mydispatcher::add_handler<pod_in<bool>>(CryptoCommands::_DEBUG_MODE, &ISecureElement::Execute_Select_Debug_Mode);
    #endif
}

void HBSETransceiver::ExitApp() {
    m_running = false;
    m_server_instance.StopServer();
    for (auto &it : m_msg_threads) {
        if (it.joinable()) {
            it.detach();
        }
    }
}

#if ENABLE_DEBUG_MODE
void HBSETransceiver::SetDebugMode(bool start_test_mode) {
    m_debug_mode = start_test_mode;
}
#endif

void HBSETransceiver::NewConnectionHandler(int fd) {

    using namespace std;

    m_executor = VirtualSecureElement::GetInstance();

    #if ENABLE_DEBUG_MODE
        if (m_debug_mode == 1)
        {
            m_executor->Execute_Select_Debug_Mode(m_debug_mode);
        }
    #endif

    std::thread t([this, fd] {

        constexpr   uint32_t io_timeout = SECONDS_TO_MS(SOCK_CONN_TIMEOUT_SEC);

        bool        io_res;
        uint32_t    io_cnt;
        USBCB       in_header;
        USBCB       out_header;
        uint8_t*    data_ptr;
        uint32_t    data_len;

        try {

            while (m_running) {

                bin_data_t msg_buffer;

                io_cnt = sizeof(in_header);

                io_res = m_server_instance.RecvFrame(fd, &in_header, io_cnt, io_timeout);

                if ( ! io_res ) {
                    err("recv USBCB header failed!");
                    continue;
                }

                io_res = vcom_hdr_validate( &in_header);

                if ( ! io_res ) {
                    err("Invalid USBCB header received!");
                    continue;
                }


                if ( in_header.ulCount > 0 ) {

                    msg_buffer.resize(in_header.ulCount);
                    io_res = m_server_instance.RecvFrame(fd, msg_buffer.data(), in_header.ulCount, io_timeout);
                    if ( ! io_res ) {
                        err("this thread of message read failed! connection closed!");
                        continue;
                    }
                }

                data_ptr  = nullptr;
                data_len  = 0;

                if ( ! msg_buffer.empty() ) {
                    data_ptr = msg_buffer.data();
                    data_len = msg_buffer.size();
                }

                auto [return_code, res_buffer] = HandleCommand(in_header.ulCommand, data_ptr, data_len);

                vcom_hdr_config ( &out_header, 0, res_buffer.size(), static_cast<uint32_t>(return_code) );

                io_cnt = sizeof(out_header);
                io_res = m_server_instance.SendFrame(fd, &out_header, io_cnt, io_timeout);

                if ( !io_res ) {
                    err("Error Failed to send resp HDR");
                    continue;
                }

                if (res_buffer.size() > 0) {

                    io_res = m_server_instance.SendFrame(fd, res_buffer.data(), res_buffer.size(), io_timeout);

                    if ( ! io_res ) {
                        err("Error Failed to send resp BODY");
                        continue;
                    }
                }
            }
        }

        catch (const std::exception & /*e*/) {
        }

    });

    m_msg_threads.emplace_back(std::move(t));
}

std::tuple<CryptExecStatus, std::vector<uint8_t>> HBSETransceiver::HandleCommand(uint32_t cmd, const uint8_t *buffer, uint32_t buffer_length) {
    return mydispatcher::dispatch((CryptoCommands)cmd, buffer, buffer_length, m_executor);
}

bool HBSETransceiver::TA_OpenSession() {
    return true;
}

bool HBSETransceiver::TA_CloseSession() {
    return true;
}
