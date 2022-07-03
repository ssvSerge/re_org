#pragma once
#include <cstdint>
#include "lumi_stdint.h"
#include <cstddef>
#include <unordered_map>
#include <cstring>

enum class HashAlgorithms : int
{
    SHA1,
    SHA2_224,
    SHA2_256,
    SHA2_384,
    SHA2_512,
    HMAC_SHA1,
    HMAC_SHA256,
    HMAC_SHA512
};

enum class KeyModes
{
    MODE_NONE      = 0,         // Placeholder
    AES_256_CBC    = 1,            // AES-256 using Cipher Block Chaining (CBC) cipher mode
    AES_128_CBC    = 2,         // AES-128 using Cipher Block Chaining (CBC) cipher mode
    TDES_ABA_ECB   = 3,         // Two-Key TDES (ABA Key Format) using Electronic Cookbook (ECB) cipher mode
    TDES_ABA_CBC   = 4,         // Two-Key TDES (ABA Key Format) using Cipher Block Chaining (CBC) cipher mode
    TDES_ABC_ECB   = 5,         // Three-Key TDES (ABC Key Format) using Electronic Cookbook (ECB) cipher mode
    TDES_ABC_CBC   = 6,         // Three-Key TDES (ABC Key Format) using Cipher Block Chaining (CBC) cipher mode
    RSA_2048_v15   = 0x1000,    // RSA-2048 (version 1.5)
    RSA_2048_v21   = 0x1001,    // RSA-2048 (version 2.1)
    DUKPT_IPEK_128 = 0x1002,    // DUKPT - Initial Pin Encryption Key
    DUKPT_KSN_64   = 0x1003,    // DUKPT - Key Serial Number
};

typedef enum
{
    KCV_NONE = 0,
    KCV_AES_256_CBC,
    KCV_SHA_256_NONE,
    KCV_AES_128_CBC,        // start new
    KCV_AES_192_CBC,
    KCV_TDES_128_CBC,        // 2 key
    KCV_TDES_192_CBC,        // 3 key
    KCV_TDES_128_ECB,        // 2 key
    KCV_TDES_192_ECB,        // 3 key
} KCV_Algorithm_Type;

struct SymmetricKeyFormat
{
    u16 version {};
    u16 mode {};
    u8 key_check_value[4] {};
    u256 key {};
};

struct AsymmetricKeyFormat
{
    u16 version {};
    u16 mode {};
    u8 key_check_value[4] {};
    u2048 key {};
};

const uint32_t kFWKeyID=0x00004657;

#pragma pack(push, 1)
struct KeyInfoStructure
{
    uint8_t key_hash[64] {};
    uint16_t slot = 0;
    uint16_t key_ver = 0;
    KeyModes key_mode = KeyModes::MODE_NONE;
    uint8_t key_check_value[4] {}; // KCV_Length = 4
    uint32_t key_size = 0;
    uint16_t rsa_sign_mode = 0; //SIG_RSA_SHA1;

    bool operator==(const KeyInfoStructure &right) const { return !memcmp(&slot, &right.slot, sizeof(right) - sizeof(key_hash)); }
    bool operator!=(const KeyInfoStructure &right) const { return !(*this == right); }
};

struct PublicRSAKeySlots
{
    uint32_t public_key_slot = 0;
    uint32_t public_key_exp_slot = 0;
};

struct DUKPTKeySlots
{
    uint32_t DUKPT_IPEK_slot = 0;
    uint32_t DUKPT_KSN_slot = 0;
    uint32_t DUKPT_TC_slot = 0;
    bool increase_transaction_counter = false;
};

// Please note: these items should be temporarily copied into memory with another variable every time it's being used.
// Due to the issue with ICypher library signatures....
constexpr unsigned char kFwKey[] = {
  0xcd, 0xef, 0x2c, 0x3b, 0xdc, 0xe3, 0xfb, 0xed, 0x13, 0x44, 0xf5, 0xf9,
  0x6e, 0x88, 0x9e, 0xeb, 0x16, 0x3e, 0x1d, 0xbf, 0xfc, 0x8c, 0xeb, 0xc3,
  0xa3, 0xbc, 0xf0, 0x36, 0x82, 0x7c, 0x42, 0x52
};
constexpr unsigned int kFwKeyLen = 32;
constexpr unsigned char kFwKeyIv[] = {
  0xb6, 0xd7, 0xe7, 0xbe, 0xb3, 0x96, 0x5f, 0xe3, 0x46, 0xaa, 0x1e, 0x5e,
  0x64, 0x3d, 0xf3, 0xf8, 0xd2, 0xfc, 0xca, 0x93, 0x4e, 0x47, 0xa4, 0x09,
  0x6c, 0xf3, 0xd0, 0x14, 0x9c, 0xdb, 0x65, 0x86
};
constexpr unsigned int kFwKeyIvLen = 32;
#pragma pack(pop)

#if __linux__
constexpr char kUpdateShmFileName[] = "/var/update_shm_file";
constexpr char kUpdateStateShmName[] = "update_state_shm";
#else
constexpr char kUpdateShmFileName[] = "";
constexpr char kUpdateStateShmName[] = "update_state_shm";
#endif

