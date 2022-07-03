#pragma once
#include "ISecureElement.h"

#include <any>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>

class SocketClient;
class test_module_log;

class HBSEClient : public ISecureElement
{
    friend HBSEClient *HBSEClient__GetInstance_();
    friend void do_ipc_test(test_module_log &log);

public:
    HBSEClient()
    {
//        fprintf(stdout, "\nConnect to server...");
        if (client_started == false)
        {
            Connect();
        }
    }
    virtual ~HBSEClient()
    {
//        fprintf(stdout, "\nDestructor, Disconnect...\n");
        Disconnect();
    }

public:
    using HBSEClientUPtrDeleter = std::function<void(HBSEClient *)>;
    using HBSEClientUPtr = std::unique_ptr<HBSEClient, HBSEClientUPtrDeleter>;
    static HBSEClientUPtr GetInstance();

    bool Connect();
    void Disconnect() override;
    bool Test(int no_more_sessions_is_success = 0); // 1 to allow no_more_sessions, 2 to require
    bool IsClientStarted() const { return client_started; }

    CryptExecStatus Execute_GetRandomBuffer(uint8_t *out_buffer, uint32_t out_buffer_len) override;
    CryptExecStatus Execute_Select_Key(uint32_t key_type) override;
    CryptExecStatus Execute_Encrypt(const uint8_t *in_buffer, uint32_t in_buffer_len, uint8_t **encrypted_out_buffer,
                                    uint32_t &encrypted_out_buffer_len, const uint8_t *iv_buffer,
                                    uint32_t iv_len) override;
    CryptExecStatus Execute_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len, uint8_t **decrypted_out_buffer,
                                    uint32_t &decrypted_out_buffer_len, const uint8_t *iv_buffer,
                                    uint32_t iv_len) override;
    // CryptExecStatus Execute_Verify_And_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
    //                                 uint8_t **decrypted_out_buffer, uint32_t &decrypted_out_buffer_len,
    //                                 const uint8_t* iv_buffer, uint32_t iv_len) override;
    virtual CryptExecStatus Execute_Verify_And_Decrypt(const uint8_t* map_filename, uint32_t map_filename_len,
                                                       const uint8_t *iv_buffer, uint32_t iv_len) override;
    CryptExecStatus Execute_Verify_And_Decrypt(std::vector<uint8_t> &&in_data_buffer, uint8_t **decrypted_out_buffer,
                                               uint32_t &decrypted_out_buffer_len, const uint8_t *iv_buffer,
                                               uint32_t iv_len);
    CryptExecStatus Execute_Hash_Data(const uint8_t *in_buffer, uint32_t in_buffer_len, HashAlgorithms hash_algorithm,
                                      uint8_t **out_hash_result, uint32_t &out_hash_result_len) override;
    CryptExecStatus Execute_Generate_RSA_Key() override;
    CryptExecStatus Execute_Reset_Secure_Element() override;
    CryptExecStatus Execute_Get_RSA_PublicKey(uint8_t **out_buffer, uint32_t &out_buffer_len) override;
    CryptExecStatus Execute_Get_Key_Info(uint32_t key_slot, KeyInfoStructure &out_key_version) override;
    CryptExecStatus Execute_Set_Key(const KeyInfoStructure &key_info, const uint8_t *in_key_buffer,
                                    uint32_t in_key_buffer_len) override;
    CryptExecStatus Execute_Erase_Key(uint32_t key_slot) override;
    CryptExecStatus Execute_Get_Status() override;
    CryptExecStatus Execute_Set_Property(const uint8_t *key, uint32_t key_length, const uint8_t *value,
                                         uint32_t value_length) override;
    CryptExecStatus Execute_Get_Property(const uint8_t *key, uint32_t key_length, uint8_t **value,
                                         uint32_t &value_length) override;
    CryptExecStatus Execute_Remove_Property(const uint8_t *key, uint32_t key_length) override;
    CryptExecStatus Execute_Check_Key_Exist(uint32_t key_slot) override;

    CryptExecStatus Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3,
                                  uint32_t &pod_out1, const uint8_t *buffer_in, uint32_t buffer_in_len,
                                  uint8_t *buffer_out, uint32_t buffer_out_len, uint8_t **allocated_buffer_out,
                                  uint32_t &allocated_buffer_out_len, const uint32_t *buffer_in2,
                                  uint32_t buffer_in2_len) override;
    CryptExecStatus Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3,
                                  uint32_t &pod_out1, const uint8_t *buffer_in, uint32_t buffer_in_len,
                                  uint8_t *buffer_out, uint32_t buffer_out_len, uint8_t **allocated_buffer_out,
                                  uint32_t &allocated_buffer_out_len, std::vector<uint32_t> &&buffer_in2);

    CryptExecStatus Execute_Select_Public_RSA_Keys(PublicRSAKeySlots slots) override;
    CryptExecStatus Execute_Select_DUKPT_Keys(DUKPTKeySlots slots) override;

    void Delete_Buffer(uint8_t **buffer) override;
#if ENABLE_DEBUG_MODE
    CryptExecStatus Execute_Select_Debug_Mode(bool enable_static = true) override;
#endif

protected:
    std::atomic_bool client_started{false};
    std::shared_ptr<SocketClient> sc;
    std::any send_to_server; // used std::any to avoid having too many includes in
                             // the header
};
