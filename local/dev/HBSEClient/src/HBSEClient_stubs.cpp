#include <chrono>
#include <thread>
#include <utility>

#include "HBSEClient.h"

HBSEClient* HBSEClient__GetInstance_() {
    static HBSEClient instance_;
    if (!instance_.client_started)
        instance_.Connect();
    return &instance_;
}

HBSEClient::HBSEClientUPtr HBSEClient::GetInstance() {
    static std::atomic<HBSEClient*> instance_ptr { HBSEClient__GetInstance_() };
    return {instance_ptr.exchange(nullptr), [instance_ptr = &instance_ptr](HBSEClient *ptr) { if (ptr) *instance_ptr = ptr; }};
}

ISecureElement *ISecureElement::GetInstance() {
    return HBSEClient__GetInstance_();
}

bool HBSEClient::Connect() {
    return client_started;
}

void HBSEClient::Disconnect() {
}

bool HBSEClient::Test(int type) {
    return true;
}

CryptExecStatus HBSEClient::Execute_GetRandomBuffer(uint8_t *out_buffer, uint32_t out_buffer_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Select_Key(uint32_t key_type) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Encrypt(const uint8_t *in_buffer, uint32_t in_buffer_len, uint8_t **encrypted_out_buffer, uint32_t &encrypted_out_buffer_len, const uint8_t *iv_buffer, uint32_t iv_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len, uint8_t **decrypted_out_buffer, uint32_t &decrypted_out_buffer_len, const uint8_t *iv_buffer, uint32_t iv_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Verify_And_Decrypt(const uint8_t* map_filename, uint32_t map_filename_len, const uint8_t* iv_buffer, uint32_t iv_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Hash_Data(const uint8_t *in_buffer, uint32_t in_buffer_len, HashAlgorithms hash_algorithm, uint8_t **out_hash_result, uint32_t &out_hash_result_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Generate_RSA_Key() {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Reset_Secure_Element() {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Get_RSA_PublicKey(uint8_t **out_buffer, uint32_t &out_buffer_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Get_Key_Info(uint32_t key_slot, KeyInfoStructure &out_key_version) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Set_Key(const KeyInfoStructure &key_info, const uint8_t *in_key_buffer, uint32_t in_key_buffer_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Erase_Key(uint32_t key_slot) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Get_Status() {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Set_Property(const uint8_t *key, uint32_t key_length, const uint8_t *value, uint32_t value_length) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Get_Property(const uint8_t *key, uint32_t key_length, uint8_t **value, uint32_t &value_length) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Remove_Property(const uint8_t *key, uint32_t key_length) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Select_Public_RSA_Keys(PublicRSAKeySlots slots) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Select_DUKPT_Keys(DUKPTKeySlots slots) {
    return CryptExecStatus::Successful;
}

void HBSEClient::Delete_Buffer(uint8_t **buffer) {
}

CryptExecStatus HBSEClient::Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3, uint32_t &pod_out1, const uint8_t *buffer_in1, uint32_t buffer_in1_len, uint8_t *buffer_out1, uint32_t buffer_out1_len, uint8_t **allocated_buffer_out1, uint32_t &allocated_buffer_out1_len, const uint32_t *buffer_in2, uint32_t buffer_in2_len) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3, uint32_t &pod_out1, const uint8_t *buffer_in1, uint32_t buffer_in1_len, uint8_t *buffer_out1, uint32_t buffer_out1_len, uint8_t **allocated_buffer_out1, uint32_t &allocated_buffer_out1_len, std::vector<uint32_t> &&buffer_in2) {
    return CryptExecStatus::Successful;
}

CryptExecStatus HBSEClient::Execute_Check_Key_Exist(uint32_t key_slot) {
    return CryptExecStatus::Successful;
}
