#include <atomic>
#include <cassert>
#include <thread>

#include "VirtualSecureElement.h"
#include "HBSETransceiver.h"

#if _WIN32
#undef SendMessage
#endif

#include "packunpack.h"
using namespace packunpack::server_side;
using mydispatcher = dispatcher<CryptoCommands, ISecureElement *, CryptExecStatus, CryptExecStatus::Invalid_Command,
                                CryptExecStatus::Execute_Error, CryptExecStatus::Execute_Exception>;


bool HBSETransceiver::StartServer()
{
    static std::once_flag once;
    std::call_once(once, [this]() { InitHandlers(); });

    m_running = true;
    auto init_socket_successful = m_server_instance.InitSocket("TACmdSocketServer", true, true, false);
    if (!init_socket_successful)
    {
        fprintf(stdout, "Error: initialize socket server failed!\n");
        return false;
    }
    std::function<MsgHandleFunctionPrototype> func =
        std::bind(&HBSETransceiver::NewConnectionHandler, this, std::placeholders::_1);
    m_server_instance.BindConnectionHandleFunction(std::move(func));
    m_server_instance.StartServer();
    return true;
}

void HBSETransceiver::InitHandlers()
{
    mydispatcher::add_handler<buffer_out<>>      
    (CryptoCommands::GET_RAND, &ISecureElement::Execute_GetRandomBuffer);

    mydispatcher::add_handler<pod_in<uint32_t>>  
    (CryptoCommands::SELECT_KEY, &ISecureElement::Execute_Select_Key);

    mydispatcher::add_handler<buffer_in<>, allocated_buffer_out<>, buffer_in<>>
    (CryptoCommands::ENCRYPT_DATA, &ISecureElement::Execute_Encrypt);

    mydispatcher::add_handler<buffer_in<>, allocated_buffer_out<>, buffer_in<>>
    (CryptoCommands::DECRYPT_DATA, &ISecureElement::Execute_Decrypt);

    mydispatcher::add_handler<buffer_in<>, buffer_in<>>
    (CryptoCommands::VERIFY_AND_DECRYPT_DATA, &ISecureElement::Execute_Verify_And_Decrypt);

    mydispatcher::add_handler<buffer_in<>, pod_in<HashAlgorithms>, allocated_buffer_out<>>
    (CryptoCommands::HASH_DATA, &ISecureElement::Execute_Hash_Data);

    mydispatcher::add_handler<>
    (CryptoCommands::GENERATE_RSA_KEY, &ISecureElement::Execute_Generate_RSA_Key);

    mydispatcher::add_handler<>
    (CryptoCommands::GENERATE_PLAT_KEYS, &ISecureElement::Execute_Reset_Secure_Element);

    mydispatcher::add_handler<allocated_buffer_out<>>
    (CryptoCommands::GET_KEY, &ISecureElement::Execute_Get_RSA_PublicKey);

    mydispatcher::add_handler<pod_in<uint32_t>, pod_out<KeyInfoStructure>>
    (CryptoCommands::GET_KEYINFO, &ISecureElement::Execute_Get_Key_Info);

    mydispatcher::add_handler<pod_in<KeyInfoStructure>, buffer_in<>>
    ( CryptoCommands::SET_KEY, static_cast<CryptExecStatus (ISecureElement::*)(const KeyInfoStructure &, const uint8_t *, uint32_t)>(&ISecureElement::Execute_Set_Key));

    mydispatcher::add_handler<pod_in<uint32_t>>
    (CryptoCommands::ERASE_KEY, &ISecureElement::Execute_Erase_Key);

    mydispatcher::add_handler<>
    (CryptoCommands::GET_STATUS, &ISecureElement::Execute_Get_Status);

    mydispatcher::add_handler<buffer_in<>, buffer_in<>>
    (CryptoCommands::SET_PROPERTY, &ISecureElement::Execute_Set_Property);

    mydispatcher::add_handler<buffer_in<>, allocated_buffer_out<>>
    (CryptoCommands::GET_PROPERTY, &ISecureElement::Execute_Get_Property);

    mydispatcher::add_handler<buffer_in<>>
    (CryptoCommands::ERASE_PROPERTY, &ISecureElement::Execute_Remove_Property);

    mydispatcher::add_handler<pod_in<uint32_t>>
    (CryptoCommands::CHECK_KEY, &ISecureElement::Execute_Check_Key_Exist);

    mydispatcher::add_handler<pod_in<DUKPTKeySlots>>
    (CryptoCommands::SELECT_DUKPT_KEY, &ISecureElement::Execute_Select_DUKPT_Keys);

    mydispatcher::add_handler<pod_in<PublicRSAKeySlots>>
    (CryptoCommands::SELECT_PUBLIC_RSA_KEY, &ISecureElement::Execute_Select_Public_RSA_Keys);

    mydispatcher::add_handler<pod_in<uint32_t>, pod_in<double>, pod_in<std::pair<uint32_t, int32_t>>, pod_out<uint32_t>, buffer_in<>, buffer_out<>, allocated_buffer_out<>, buffer_in<uint32_t>>
    (CryptoCommands::_TEST, &ISecureElement::Execute__test);

    #if ENABLE_DEBUG_MODE
        mydispatcher::add_handler<pod_in<bool>>(CryptoCommands::_DEBUG_MODE, &ISecureElement::Execute_Select_Debug_Mode);
    #endif
}

void HBSETransceiver::ExitApp()
{
    m_running = false;
    m_server_instance.StopServer();
    for (auto &it : m_msg_threads)
    {
        if (it.joinable())
        {
            it.detach();
        }
    }
}

#if ENABLE_DEBUG_MODE
void HBSETransceiver::SetDebugMode(bool start_test_mode)
{
    m_debug_mode = start_test_mode;
}
#endif

void HBSETransceiver::NewConnectionHandler(int conn_file_descriptor)
{
    using namespace std;
    m_executor = VirtualSecureElement::GetInstance();
#if ENABLE_DEBUG_MODE
    if (m_debug_mode == 1)
    {
        m_executor->Execute_Select_Debug_Mode(m_debug_mode);
    }
#endif

    std::thread t([this, conn_file_descriptor] {
        std::vector<uint8_t> msg_buffer;
        uint8_t *response_buffer = nullptr;
        uint32_t response_buffer_len = 0;

        constexpr uint32_t timeout_ms = 1000;

        bool session_opened = false;
        try
        {
            while (m_running)
            {
                msg_buffer.clear();

                USBCB header{};
                uint32_t header_size = sizeof(header);

                auto socket_success =
                    m_server_instance.RecvMessage(conn_file_descriptor, (uint8_t *)&header, header_size, timeout_ms);
                if (!socket_success || header_size != sizeof(header))
                {
                    fprintf(stdout, "recv USBCB header failed!\n");
                    break;
                }
                msg_buffer.resize(header.ulCount);
                if (header.ulCount > 0)
                {
                    socket_success = m_server_instance.RecvMessage(conn_file_descriptor, msg_buffer.data(),
                                                                   header.ulCount, timeout_ms);
                    assert(header.ulCount == msg_buffer.size());
                    if (!socket_success || (header.ulCount != msg_buffer.size()))
                    {
                        fprintf(stdout, "this thread of message read failed! connection closed!\n");
                        break;
                    }
                }

                if (!session_opened)
                    session_opened = TA_OpenSession();
                if (!session_opened)
                    fprintf(stdout, "Error: no more sessions!\n");

                auto [return_code, response_buffer] = session_opened
                    ? HandleCommand(header.ulCommand, msg_buffer.empty() ? nullptr : msg_buffer.data(),
                                    msg_buffer.size())
                    : std::make_tuple(CryptExecStatus::No_More_Sessions, std::vector<uint8_t>());

                header.ulData = (uint32_t)return_code;
                header.ulCount = response_buffer.size();
                header_size = sizeof(header);
                socket_success = m_server_instance.SendMessage(conn_file_descriptor, &header, header_size, timeout_ms);
                if (!socket_success || header_size != sizeof(header))
                {
                    fprintf(stdout, "Error: send message body failed! other side may not exist!\n");
                    break;
                }
                if (header.ulCount > 0)
                {
                    socket_success = m_server_instance.SendMessage(conn_file_descriptor, response_buffer.data(),
                                                                   header.ulCount, timeout_ms);
                    if (!socket_success || (header.ulCount != response_buffer.size()))
                    {
                        fprintf(stdout, "Error: send message body failed! other side may not exist!\n");
                        break;
                    }
                }
            }
        }
        catch (const std::exception & /*e*/)
        {
        }

        if (session_opened)
            TA_CloseSession();
    });
    m_msg_threads.emplace_back(std::move(t));
}

std::tuple<CryptExecStatus, std::vector<uint8_t>> HBSETransceiver::HandleCommand(uint32_t cmd, const uint8_t *buffer,
                                                                                 uint32_t buffer_length)
{
    return mydispatcher::dispatch((CryptoCommands)cmd, buffer, buffer_length, m_executor);
}

static std::atomic_bool session_opened{false};
static thread_local bool session_is_mine = false;

bool HBSETransceiver::TA_OpenSession()
{
    // printf("TA_OpenSession\n");

    if (session_opened.exchange(true))
        return session_is_mine; // already opened
    assert(!session_is_mine);

    if (m_executor->Execute_Select_Key(0) != CryptExecStatus::Successful)
    {
        //        session_opened = false;
        //        return false;
    }
    session_is_mine = true;
    return true;
}

bool HBSETransceiver::TA_CloseSession()
{
    // printf("TA_CloseSession\n");

    assert(session_is_mine);
    if (!session_is_mine)
        return false;
    assert(session_opened);

    bool ret = m_executor->Execute_Select_Key(0) != CryptExecStatus::Successful;
    session_is_mine = false;
    session_opened = false;
    return /*ret*/ true;
}