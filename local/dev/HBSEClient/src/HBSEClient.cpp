#include "global.h"

TDBG_DEFINE_AREA(seclient);

#include <chrono>
#include <thread>
#include <utility>

#include <packunpack.h>
#include <SocketClient.h>
#include <HBSEClient.h>

#include <application/const.h>
#include <application/types_c.h>
#include <application/vcom_tools.h>

// #include <logging.h>

// #define info     printf
// #define err      printf

using namespace packunpack::client_side;

using IPCDispatcher     = dispatcher<CryptoCommands, CryptExecStatus, CryptExecStatus::Execute_Error, CryptExecStatus::Execute_Exception, CryptExecStatus::No_More_Sessions>;
using send_to_server_t  = std::function<IPCDispatcher::reply_t(CryptoCommands, std::vector<uint8_t> &&)>;

const send_to_server_t *get(const std::any &send_to_server) {
    assert(!send_to_server.has_value() || (send_to_server.type() == typeid(send_to_server_t)));
    return std::any_cast<send_to_server_t>(&send_to_server);
}

HBSEClient *HBSEClient__GetInstance_() {

    static HBSEClient instance_;

    if (!instance_.client_started) {
        instance_.Connect();
    }

    return &instance_;
}

HBSEClient::HBSEClientUPtr HBSEClient::GetInstance() {

    static std::atomic<HBSEClient *> instance_ptr { HBSEClient__GetInstance_() };

    return { instance_ptr.exchange(nullptr), [instance_ptr = &instance_ptr](HBSEClient *ptr) 
    { if (ptr) { *instance_ptr = ptr; } }};
}

ISecureElement *ISecureElement::GetInstance() {
    return HBSEClient__GetInstance_();
}

#if ENABLE_DEBUG_MODE 
CryptExecStatus HBSEClient::Execute_Select_Debug_Mode(bool enable_static) {
    IPCDispatcher::SendToServer(CryptoCommands::_DEBUG_MODE, get(send_to_server), pod_in(enable_static));
}
#endif

bool HBSEClient::Connect() {

    if (client_started) {
        return true;
    }

    sc = std::make_shared<SocketClient>();

    // Try to create socket.
    for (int retry_cnt = 0; retry_cnt < SOCK_CONN_RETRY_CNT; retry_cnt++) {
        client_started = sc->InitSocket(VCOM_TACMD_SOCK_NAME, true, false);
        if ( client_started ) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SOCK_CONN_RETRY_CNT));
    }

    if ( ! client_started ) {
        // err("\nCannot initialize socket client!");
        return false;
    }

    // Initialization still not done.
    client_started = false;

    for (int retry_cnt = 0; retry_cnt < SOCK_CONN_RETRY_CNT; retry_cnt++) {

        client_started = sc->StartClient();

        if (client_started) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SOCK_CONN_RETRY_CNT));
    }

    if ( ! client_started ) { 
        // err("\nCannot establish socket communication to server-side!");
        return false;
    }

    send_to_server.emplace<send_to_server_t>(

        [this](CryptoCommands cmd, std::vector<uint8_t> &&message_buffer) -> IPCDispatcher::reply_t {
    
            constexpr   uint32_t io_timeout = SECONDS_TO_MS ( SOCK_CONN_TIMEOUT_SEC );
            bool        io_res;
            uint32_t    io_cnt;

            USBCB out_header{};
            USBCB inp_header{};

            assert(client_started && sc);

            if ( ! client_started ) {
                // err("Client not started.");
                return { CryptExecStatus::Comm_Error, {} };
            }

            if ( nullptr == sc ) {
                // err ("Client sock is NULL");
                return {CryptExecStatus::Comm_Error, {}};
            }

            io_cnt = message_buffer.size();

            vcom_hdr_config ( &out_header, static_cast<int>(cmd), io_cnt, 0 );

            io_res = sc->SendFrame( &out_header, sizeof(out_header), io_timeout);

            if ( ! io_res ) {
                // err ("Failed to send CMD header.");
                return {CryptExecStatus::Comm_Error, {}};
            }

            if (io_cnt > 0 ) {
                io_res = sc->SendFrame(message_buffer.data(), io_cnt, io_timeout);
                if ( ! io_res ) {
                    // err("Failed to send CMD body.");
                    return {CryptExecStatus::Comm_Error, {}};
                }
            }

            message_buffer.clear();

            io_cnt = sizeof(inp_header);
            io_res = sc->RecvFrame(&inp_header, io_cnt, io_timeout);

            if ( ! io_res ) {
                // err ("Cannot receive CMD header");
                return { CryptExecStatus::Comm_Error, {} };
            }

            io_res = vcom_hdr_validate ( &inp_header );

            if ( ! io_res ) {
                // err("Invalid CMD header");
                return { CryptExecStatus::Comm_Error, {} };
            }

            io_cnt = inp_header.ulCount;

            if ( io_cnt > 0 ) {

                message_buffer.resize(io_cnt);

                io_res = sc->RecvFrame(message_buffer.data(), io_cnt, io_timeout);

                if ( ! io_res ) {
                    // err("Cannot receive Response");
                    return { CryptExecStatus::Comm_Error, {} };
                }

            }

            return {(CryptExecStatus)inp_header.ulData, std::move(message_buffer)};

        });

    return client_started;
}

void HBSEClient::Disconnect() {

    // info ("Disconnect requested (%s):(%d)", __FUNCTION__, __LINE__);

    client_started = false;
    send_to_server.reset();
    if (sc) {
        sc->StopClient();
        sc.reset();
    }
}

bool HBSEClient::Test(int type) {

    // type:
    //   0 - normal test
    //   1 - tolerate No_More_sessions
    //   2 - only accept No_More_sessions
    //   3 - only accept Execute_Exception
    //   4 - initate crash

    assert(IsClientStarted());
    if (!IsClientStarted())
        return false;

    static const uint8_t buffer_in1[8] = {100, 101, 102, 103, 104, 105, 106, 107};

    uint32_t out1 = 0;
    uint8_t buffer_out2[33] = {0};
    uint8_t *allocated_buffer_out1 = nullptr;
    uint32_t allocated_buffer_out1_len = 0;

    std::vector<uint32_t> buffer_in2;
    for (int i = 0; i < 128; i++)
        buffer_in2.emplace_back(100000 + i);

    CryptExecStatus ret = Execute__test((type == 4) ? 1999 : 1234, (type == 4) ? 11.11 : 56.78,
                                        std::make_pair(10u, -10), out1, buffer_in1, 8, buffer_out2, 33,
                                        &allocated_buffer_out1, allocated_buffer_out1_len, std::move(buffer_in2));
    if (type > 0)
    {
        if ((type == 3) || (type == 4))
            return ret == CryptExecStatus::Execute_Exception;

        assert((type < 2) || (ret == CryptExecStatus::No_More_Sessions));
        if (ret == CryptExecStatus::No_More_Sessions)
            return true;
        if (type == 2)
            return false;
    }
    assert(ret == CryptExecStatus::Successful);
    if (ret != CryptExecStatus::Successful)
        return false;

    for (uint32_t i = 0; i < 33; i++)
    {
        assert(buffer_out2[i] == (200 + i));
        if (buffer_out2[i] != (200 + i))
            return false;
    }

    for (uint32_t i = 0; i < allocated_buffer_out1_len; i++)
    {
        assert(allocated_buffer_out1[i] == (50 + i));
        if (allocated_buffer_out1[i] != (50 + i))
            return false;
    }
    Delete_Buffer(&allocated_buffer_out1);

    return true;

    // declare a buffer
    //    uint8_t *byte_rand_buffer = new uint8_t[32];
    //    CryptExecStatus execute_result =
    //    Execute_GetRandomBuffer(byte_rand_buffer, 32);
    // Now it's ready to use.
}

CryptExecStatus HBSEClient::Execute_GetRandomBuffer(uint8_t *out_buffer, uint32_t out_buffer_len) {
    return IPCDispatcher::SendToServer(CryptoCommands::GET_RAND, get(send_to_server), buffer_out(out_buffer, out_buffer_len));
}

CryptExecStatus HBSEClient::Execute_Select_Key(uint32_t key_type) {
    return IPCDispatcher::SendToServer(CryptoCommands::SELECT_KEY, get(send_to_server), pod_in(key_type));
}

CryptExecStatus HBSEClient::Execute_Encrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                            uint8_t **encrypted_out_buffer, uint32_t &encrypted_out_buffer_len,
                                            const uint8_t *iv_buffer, uint32_t iv_len)
{
    return IPCDispatcher::SendToServer(
        CryptoCommands::ENCRYPT_DATA, get(send_to_server), buffer_in(in_buffer, in_buffer_len),
        allocated_buffer_out(encrypted_out_buffer, encrypted_out_buffer_len), buffer_in(iv_buffer, iv_len));
}

CryptExecStatus HBSEClient::Execute_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                            uint8_t **decrypted_out_buffer, uint32_t &decrypted_out_buffer_len,
                                            const uint8_t *iv_buffer, uint32_t iv_len)
{
    return IPCDispatcher::SendToServer(
        CryptoCommands::DECRYPT_DATA, get(send_to_server), buffer_in(in_buffer, in_buffer_len),
        allocated_buffer_out(decrypted_out_buffer, decrypted_out_buffer_len), buffer_in(iv_buffer, iv_len));
}

CryptExecStatus HBSEClient::Execute_Verify_And_Decrypt(const uint8_t* map_filename, uint32_t map_filename_len,
                                                       const uint8_t* iv_buffer, uint32_t iv_len)
{
    return IPCDispatcher::SendToServer(CryptoCommands::VERIFY_AND_DECRYPT_DATA, get(send_to_server),
                                       buffer_in(map_filename, map_filename_len), buffer_in(iv_buffer, iv_len));
}

CryptExecStatus HBSEClient::Execute_Hash_Data(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                              HashAlgorithms hash_algorithm, uint8_t **out_hash_result,
                                              uint32_t &out_hash_result_len)
{
    return IPCDispatcher::SendToServer(CryptoCommands::HASH_DATA, get(send_to_server),
                                       buffer_in(in_buffer, in_buffer_len), pod_in(hash_algorithm),
                                       allocated_buffer_out(out_hash_result, out_hash_result_len));
}

CryptExecStatus HBSEClient::Execute_Generate_RSA_Key() {
    return IPCDispatcher::SendToServer(CryptoCommands::GENERATE_RSA_KEY, get(send_to_server));
}

CryptExecStatus HBSEClient::Execute_Reset_Secure_Element() {
    return IPCDispatcher::SendToServer(CryptoCommands::GENERATE_PLAT_KEYS, get(send_to_server));
}

CryptExecStatus HBSEClient::Execute_Get_RSA_PublicKey(uint8_t **out_buffer, uint32_t &out_buffer_len) {
    return IPCDispatcher::SendToServer(CryptoCommands::GET_KEY, get(send_to_server), allocated_buffer_out(out_buffer, out_buffer_len));
}

CryptExecStatus HBSEClient::Execute_Get_Key_Info(uint32_t key_slot, KeyInfoStructure &out_key_version) {
    return IPCDispatcher::SendToServer(CryptoCommands::GET_KEYINFO, get(send_to_server), pod_in(key_slot), pod_out(out_key_version));
}

CryptExecStatus HBSEClient::Execute_Set_Key(const KeyInfoStructure &key_info, const uint8_t *in_key_buffer, uint32_t in_key_buffer_len) {
    return IPCDispatcher::SendToServer(CryptoCommands::SET_KEY, get(send_to_server), pod_in(key_info), buffer_in(in_key_buffer, in_key_buffer_len));
}

CryptExecStatus HBSEClient::Execute_Erase_Key(uint32_t key_slot) {
    return IPCDispatcher::SendToServer(CryptoCommands::ERASE_KEY, get(send_to_server), pod_in(key_slot));
}

CryptExecStatus HBSEClient::Execute_Get_Status() {
    return IPCDispatcher::SendToServer(CryptoCommands::GET_STATUS, get(send_to_server));
}

CryptExecStatus HBSEClient::Execute_Set_Property(const uint8_t *key, uint32_t key_length, const uint8_t *value, uint32_t value_length) {
    return IPCDispatcher::SendToServer(CryptoCommands::SET_PROPERTY, get(send_to_server), buffer_in(key, key_length), buffer_in(value, value_length));
}

CryptExecStatus HBSEClient::Execute_Get_Property(const uint8_t *key, uint32_t key_length, uint8_t **value, uint32_t &value_length) {
    return IPCDispatcher::SendToServer(CryptoCommands::GET_PROPERTY, get(send_to_server), buffer_in(key, key_length), allocated_buffer_out(value, value_length));
}

CryptExecStatus HBSEClient::Execute_Remove_Property(const uint8_t *key, uint32_t key_length) {
    return IPCDispatcher::SendToServer(CryptoCommands::ERASE_PROPERTY, get(send_to_server), buffer_in(key, key_length));
}

CryptExecStatus HBSEClient::Execute_Select_Public_RSA_Keys(PublicRSAKeySlots slots) {
    return IPCDispatcher::SendToServer(CryptoCommands::SELECT_PUBLIC_RSA_KEY, get(send_to_server), pod_in(slots));
}

CryptExecStatus HBSEClient::Execute_Select_DUKPT_Keys(DUKPTKeySlots slots) {
    return IPCDispatcher::SendToServer(CryptoCommands::SELECT_DUKPT_KEY, get(send_to_server), pod_in(slots));
}

void HBSEClient::Delete_Buffer(uint8_t **buffer)
{
    if (buffer && *buffer)
    {
        delete[] * buffer;
        *buffer = nullptr;
    }
}

CryptExecStatus HBSEClient::Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3,
                                          uint32_t &pod_out1, const uint8_t *buffer_in1, uint32_t buffer_in1_len,
                                          uint8_t *buffer_out1, uint32_t buffer_out1_len,
                                          uint8_t **allocated_buffer_out1, uint32_t &allocated_buffer_out1_len,
                                          const uint32_t *buffer_in2, uint32_t buffer_in2_len)
{
    return IPCDispatcher::SendToServer(
        CryptoCommands::_TEST, get(send_to_server), pod_in(pod_in1), pod_in(pod_in2), pod_in(pod_in3),
        pod_out(pod_out1), buffer_in(buffer_in1, buffer_in1_len), buffer_out(buffer_out1, buffer_out1_len),
        allocated_buffer_out(allocated_buffer_out1, allocated_buffer_out1_len), buffer_in(buffer_in2, buffer_in2_len));
}

CryptExecStatus HBSEClient::Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3,
                                          uint32_t &pod_out1, const uint8_t *buffer_in1, uint32_t buffer_in1_len,
                                          uint8_t *buffer_out1, uint32_t buffer_out1_len,
                                          uint8_t **allocated_buffer_out1, uint32_t &allocated_buffer_out1_len,
                                          std::vector<uint32_t> &&buffer_in2)
{
    return IPCDispatcher::SendToServer(
        CryptoCommands::_TEST, get(send_to_server), pod_in(pod_in1), pod_in(pod_in2), pod_in(pod_in3),
        pod_out(pod_out1), buffer_in(buffer_in1, buffer_in1_len), buffer_out(buffer_out1, buffer_out1_len),
        allocated_buffer_out(allocated_buffer_out1, allocated_buffer_out1_len), buffer_in(std::move(buffer_in2)));
}

CryptExecStatus HBSEClient::Execute_Check_Key_Exist(uint32_t key_slot) {
    return IPCDispatcher::SendToServer(CryptoCommands::CHECK_KEY, get(send_to_server), pod_in(key_slot));
}
