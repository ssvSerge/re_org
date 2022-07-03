#pragma once
#include "CommonCypherTypes.h"
#include "ICypher.h"
#include "ISecureElement.h"
#include <cstddef>
#include <new>
#include <exception>
#include <vector>

template <typename EnumType, EnumType... Values>
class EnumCheck;

template <typename EnumType>
class EnumCheck<EnumType> {
public:
    template <typename IntType>
    static bool constexpr is_value(IntType) {
        return false;
    }
};

template <typename EnumType, EnumType V, EnumType... Next>
class EnumCheck<EnumType, V, Next...> : private EnumCheck<EnumType, Next...> {
    using super = EnumCheck<EnumType, Next...>;

public:
    template <typename IntType>
    static bool constexpr is_value(IntType v) {
        return v == static_cast<IntType>(V) || super::is_value(v);
    }
};

class VirtualSecureElement : public ISecureElement {
public:
    VirtualSecureElement();
    virtual ~VirtualSecureElement() = default;

    /*
        Execute_GetRandomBuffer
        Populates buffer with random data
    */
    CryptExecStatus Execute_GetRandomBuffer(uint8_t *out_buffer, uint32_t out_buffer_len) override;
    /*
        Execute_Select_Key
        Updates active context to use the key located at the provided key slot
    */
    CryptExecStatus Execute_Select_Key(uint32_t key_slot) override;
    /*
        Execute_Encrypt
        Encrypts the input buffer using the active context
    */
    CryptExecStatus Execute_Encrypt(const uint8_t *in_buffer, uint32_t in_buffer_len, uint8_t **encrypted_out_buffer,
                                    uint32_t &encrypted_out_buffer_len, const uint8_t *iv_buffer,
                                    uint32_t iv_len) override;
    /*
        Execute_Decrypt
        Decrypts the input buffer using the active context
    */
    CryptExecStatus Execute_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len, uint8_t **decrypted_out_buffer,
                                    uint32_t &decrypted_out_buffer_len, const uint8_t *iv_buffer,
                                    uint32_t iv_len) override;
    // CryptExecStatus Execute_Verify_And_Decrypt(const uint8_t *in_buffer, uint32_t in_buffer_len,
    //                                            uint8_t **decrypted_out_buffer, uint32_t &decrypted_out_buffer_len,
    //                                            const uint8_t *iv_buffer, uint32_t iv_len) override;
    /*
        Execute_Verify_And_Decrypt
        Validates firmware update packet and decrypts if valid
    */
    CryptExecStatus Execute_Verify_And_Decrypt(const uint8_t* map_filename, uint32_t map_filename_len,
                                               const uint8_t *iv_buffer, uint32_t iv_len) override;
    /*
        Execute_Hash_Data
        Hashes the input buffer using the algorithm specified
        NOTE: Some algorithms will utilize the active context
    */
    CryptExecStatus Execute_Hash_Data(const uint8_t *in_buffer, uint32_t in_buffer_len, HashAlgorithms hash_algorithm,
                                      uint8_t **out_hash_result, uint32_t &out_hash_result_len) override;
    /*
        Execute_Generate_RSA_Key
        Generates RSA keys for the device
    */
    CryptExecStatus Execute_Generate_RSA_Key() override;
    /*
        Execute_Reset_Secure_Element
        Generates new master and filesystem keys
    */
    CryptExecStatus Execute_Reset_Secure_Element() override;
    /*
        Execute_Get_RSA_PublicKey
        Reads and returns the device public RSA key
    */
    CryptExecStatus Execute_Get_RSA_PublicKey(uint8_t **out_buffer, uint32_t &out_buffer_len) override;
    /*
        Execute_Get_Key_Info
        Returns key information for the key located at the key slot provided
        NOTE: The key to retrieve info for does not need to be loaded in active context
    */
    CryptExecStatus Execute_Get_Key_Info(uint32_t key_slot, KeyInfoStructure &out_key_version) override;
    /*
        Execute_Set_Key
        Saves the key to the key slot provided
        NOTE: If a key using the key slot provided is loaded in the active context, the active context will be invalidated
    */
    CryptExecStatus Execute_Set_Key(const KeyInfoStructure &in_key_info, const uint8_t *in_key_buffer,
                                    uint32_t in_key_buffer_len) override;
    /*
        Execute_Erase_Key
        Erases the key located at the provided key slot
        NOTE: If a key using the key slot provided is loaded in the active context, the active context will be invalidated
    */
    CryptExecStatus Execute_Erase_Key(uint32_t key_slot) override;
    /*
        Execute_Get_Status
        Checks if the master and filesystem keys exist
    */
    CryptExecStatus Execute_Get_Status() override;
    /*
        Execute_Set_Property
        Saves a key (identifier) and property pair to storage
    */
    CryptExecStatus Execute_Set_Property(const uint8_t *key, uint32_t key_length, const uint8_t *value,
                                         uint32_t value_length) override;
    /*
        Execute_Get_Property
        Retrieves data saved under the key (identifier) provided from storage
    */
    CryptExecStatus Execute_Get_Property(const uint8_t *key, uint32_t key_length, uint8_t **value,
                                         uint32_t &value_length) override;
    /*
        Execute_Remove_Property
        Erases a key (identifier) and property pair from storage
    */
    CryptExecStatus Execute_Remove_Property(const uint8_t *key, uint32_t key_length) override;
    /*
        Execute_Check_Key_Exist
        Checks if a key exists at the key slot provided
        NOTE: The key does not need to be loaded in the active context
    */
    CryptExecStatus Execute_Check_Key_Exist(uint32_t key_slot) override;
    /*
        Execute_Select_Public_RSA_Keys
        Updates active context to use the public RSA key(s) located at the provided key slot(s)
    */
    CryptExecStatus Execute_Select_Public_RSA_Keys(PublicRSAKeySlots slots) override;
    /*
        Execute_Select_DUKPT_Keys
        Updates active context to use the DUKPT key(s) located at the provided key slot(s)
    */
    CryptExecStatus Execute_Select_DUKPT_Keys(DUKPTKeySlots slots) override;

    CryptExecStatus Execute__test(int32_t pod_in1, double pod_in2, const std::pair<uint32_t, int32_t> &pod_in3,
                                  uint32_t &pod_out1, const uint8_t *buffer_in, uint32_t buffer_in_len,
                                  uint8_t *buffer_out, uint32_t buffer_out_len, uint8_t **allocated_buffer_out,
                                  uint32_t &allocated_buffer_out_len, const uint32_t *buffer_in2,
                                  uint32_t buffer_in2_len) override;

    /*
        Delete_Buffer
        Deletes dynamically allocated memory that was allocated by the VirtualSecureElement
    */
    void Delete_Buffer(uint8_t **buffer) override;
    void Disconnect() override;
#if ENABLE_DEBUG_MODE
    CryptExecStatus Execute_Select_Debug_Mode(bool enable_static = true) override;
#endif
    void SetDebugMode();

private:
    void InvalidateContext();
    CryptExecStatus IncActiveDUKPTCounter(uint32_t nSlot);
    CryptExecStatus LocalGetKeySlot(KeyInfoStructure &key_info, u8 **pKey);
    CryptExecStatus LocalSetKeySlot(const KeyInfoStructure &key_info, const u8 *pKey);

    bool CreateKCV(u8 *pKey, uint nKeySize, uint nZeros, uint nVals, u8 *pKCV, int nKCVType);
    bool WriteKeySlot(uint8_t *pKeyData, uint nSize, uint nKeySlot);
    bool ReadKeySlot(uint8_t **pKeyData, uint *nSize, uint nKeySlot);
    bool EraseKeySlot(uint nKeySlot);
    bool GetFSKey(u256 &pKey);
    // bool SetFSKey(u256 pKey);
    bool IsSEKeyLoaded(u16 nSlot);
    bool IsKeyExist(u16 nSlot);
    // u32 GetKeySize(u16 nSlot, u16 nKeyMode);
    //int GetKCVType(u16 nSlot, u16 nKeyMode);
    std::string GetHashStrFromBuffer(const uint8_t *buffer, uint32_t buffer_len);

    bool Encrypt(u8 *pKey, uint nKeySize, u16 nKeyMode, u8 *pIn, uint nInSize, u8 **pOutCG, uint &nOutCGSize);
    bool Decrypt(u8 *pKey, uint nKeySize, u16 nKeyMode, u8 *pInCG, uint nInCGSize, u8 *pOut, uint *nOutSize);
    bool GetCryptoAlgo(u16 nKeyMode, int &nAlgoMode, ICRYPTOAlgo **pCryptoAlgo);
    bool DeviceRSAKeyDecrypt(const u8* in_buffer, u32 in_len, u8** out_buffer, u32& out_bytes);
    bool PKIEncrypt(const u8* in_buffer, u32 in_len, u8** out_buffer, u32& out_bytes);

    std::vector<uint32_t> m_current_context_slots;
    u8 *m_current_key_value = nullptr;
    KeyInfoStructure m_current_key_info;
    u8 *m_dukpt_ksn_value = nullptr;
    u8 *m_dukpt_tc_value = nullptr;
    u8 *m_rsa_public_exp = nullptr;
#if ENABLE_DEBUG_MODE
    bool debug_mode_flag = false;
#endif

    u256 ANBIO {};
};

union UnionIntBytesConvert
{
    int n;
    uint8_t bytes[4];
};

enum class RSA_PKCS_VERS : int
{
    V15 = 0,
    V21
};

enum class RSA_KeyTypes
{
    PUBLIC = 0,
    PRIVATE
};
