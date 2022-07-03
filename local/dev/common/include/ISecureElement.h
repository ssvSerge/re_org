#pragma once
#include "CommonCypherTypes.h"
#include "VirtualCryptCommon.h"


class ISecureElement
{
public:
    virtual ~ISecureElement() = default;

    virtual CryptExecStatus Execute_GetRandomBuffer(uint8_t *out_buffer, uint32_t out_buffer_len)   = 0;
    virtual CryptExecStatus Execute_Select_Key(uint32_t key_slot)                                   = 0;
    virtual CryptExecStatus Execute_Encrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                            uint8_t **encrypted_out_buffer, uint32_t &encrypted_out_buffer_len,
                                            const uint8_t* iv_buffer, uint32_t iv_len)              = 0;
    virtual CryptExecStatus Execute_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                            uint8_t **decrypted_out_buffer, uint32_t &decrypted_out_buffer_len,
                                            const uint8_t* iv_buffer, uint32_t iv_len)              = 0;
    virtual CryptExecStatus Execute_Verify_And_Decrypt(const uint8_t* map_filename, uint32_t map_filename_len,
                                                       const uint8_t *iv_buffer, uint32_t iv_len)   = 0;
    virtual CryptExecStatus Execute_Hash_Data(const uint8_t *in_buffer, uint32_t in_buffer_len,
                                              HashAlgorithms hash_algorithm, uint8_t **out_hash_result,
                                              uint32_t &out_hash_result_len)                        = 0;
    virtual CryptExecStatus Execute_Generate_RSA_Key()                                              = 0;
    virtual CryptExecStatus Execute_Reset_Secure_Element()                                          = 0;
    virtual CryptExecStatus Execute_Get_RSA_PublicKey(uint8_t **out_buffer,
                                                      uint32_t &out_buffer_len)                     = 0;

    virtual CryptExecStatus Execute_Get_Key_Info(uint32_t key_slot, KeyInfoStructure &out_key_version) = 0;

    virtual CryptExecStatus Execute_Set_Key(const KeyInfoStructure &key_info, const uint8_t *in_key_buffer,
                                            uint32_t in_key_buffer_len)                             = 0;
    virtual CryptExecStatus Execute_Erase_Key(uint32_t key_slot)                                    = 0;
    virtual CryptExecStatus Execute_Get_Status()                                                    = 0;
    // Hash the key, save the value with encrypted, then return.
    virtual CryptExecStatus Execute_Set_Property(const uint8_t *key, uint32_t key_length, const uint8_t *value,
                                                 uint32_t value_length)                             = 0; // Encrypted with kFS
    virtual CryptExecStatus Execute_Get_Property(const uint8_t *key, uint32_t key_length, uint8_t **value,
                                                 uint32_t &value_length)                            = 0; // Encrypted with kFS
    virtual CryptExecStatus Execute_Remove_Property(const uint8_t *key, uint32_t key_length) = 0;
    virtual CryptExecStatus Execute_Check_Key_Exist(uint32_t key_slot)                              = 0;

    virtual CryptExecStatus Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3,
                                          uint32_t &pod_out1, const uint8_t *buffer_in, uint32_t buffer_in_len,
                                          uint8_t *buffer_out, uint32_t buffer_out_len, uint8_t **allocated_buffer_out,
                                          uint32_t &allocated_buffer_out_len, const uint32_t *buffer_in2,
                                          uint32_t buffer_in2_len)                                  = 0;

    virtual CryptExecStatus Execute_Select_Public_RSA_Keys(PublicRSAKeySlots slots)                 = 0;
    virtual CryptExecStatus Execute_Select_DUKPT_Keys(DUKPTKeySlots slots)                          = 0;

    virtual void Delete_Buffer(uint8_t **buffer)                                                    = 0;
    virtual void Disconnect()                                                                       = 0;

#if ENABLE_DEBUG_MODE
    virtual CryptExecStatus Execute_Select_Debug_Mode(bool enable_static)                           = 0;
#endif

    template <typename T>
    CryptExecStatus Execute_GetRandomNumber(T &target_number)
    {
        T buffer;
        const uint32_t buffer_len = (uint32_t)sizeof(T);
        return Execute_GetRandomBuffer((uint8_t *)&target_number, buffer_len);
    }
    CryptExecStatus Execute_Set_Key(const KeyInfoStructure &in_key_info, const uint8_t *in_key_buffer)
    {
        return Execute_Set_Key(in_key_info, in_key_buffer, in_key_info.key_size);
    }

    static ISecureElement *GetInstance();
};