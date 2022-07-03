#pragma once

// Shared Definitions

#include "lumi_stdint.h"
// Make sure this is in synch with _V100_ENC_KEY_TYPE
typedef enum
{
    EXTKEY_VEND               =  0,
    EXTKEY_CTK                =  1,
    EXTKEY_BTK                =  2,
    EXTKEY_AES0               =  3,
    EXTKEY_AES1               =  4,
    EXTKEY_AES2               =  5,
    EXTKEY_AES3               =  6,
    EXTKEY_TDES0              =  7,
    EXTKEY_TDES1              =  8,
    EXTKEY_TDES2              =  9,
    EXTKEY_TDES3              = 10,
    EXTKEY_AES_VEND           = 11,
    EXTKEY_KSN_0              = 12,
    EXTKEY_KSN_1              = 13,
    EXTKEY_SPARE_2            = 14,
    EXTKEY_SPARE_3            = 15,
    EXTKEY_HOST_PUBLIC        = 16,
    EXTKEY_DEVICE_PUBLIC      = 17,
    EXTKEY_DEVICE_PRIVATE     = 18,
    EXTKEY_DEVICE_P           = 19,
    EXTKEY_DEVICE_Q           = 20,
    EXTKEY_PUBLIC_EXP         = 21,
    EXTKEY_PRIVATE_EXP        = 22,
    EXT_LAST                  = 23
} EXTKeyNameType;


// These map the original key map (see KeyNameType) into
// the EXT key space defined above, and also defined in
// in the public-facing header V100_enc_types.h as _V100_ENC_KEY_TYPE.
//
// In other words, strip the high bit from a value of
// type _V100_ENC_KEY_TYPE in this range, and it becomes
// the key identifier in the original key space.
//
#define MSK_ID_MASK            0x2000
#define    IS_MSK_KEY(x)        ((x & MSK_ID_MASK) == MSK_ID_MASK)
#define    MSK_KEY_SLOT(x)        (x & ~MSK_ID_MASK)


/*
**  Secure SRAM Key Storage Map Indices (BDB style)
*/
typedef enum
{
    MKPRIME_ID = 0,        //
    MKD_ID,            // Diversified Master Key
    TK_ID,            // Transport Key
    SK_ID,            // Session Key
    TMP_ID,            // Temp Key
    TEST_ID,
    LUMIKEY_ID,            // Lumi Device Transport Key
    RESERVED_0,
    MKD_INFO_ID,
    SK_INFO_ID,
    RESERVED_3,
    RESERVED_4,
    RESERVED_5,
    RESERVED_6,
    RESERVED_7,
    RESERVED_8,            // ONLY Key that is R/W
   LAST_SRAM_KEY
} KeyNameType;

/*
**
**    Define KeyMap for KeyState member of BDB style KeyStatusType
**
*/
#define MKPRIME_ID_KEY_IS_SET                (1<<MKPRIME_ID)
#define MKD_ID_KEY_IS_SET                (1<<MKD_ID)
#define TK_ID_KEY_IS_SET                (1<<TK_ID)
#define SK_ID_KEY_IS_SET                (1<<SK_ID)
#define TMP_ID_KEY_IS_SET                (1<<TMP_ID)
#define TEST_ID_KEY_IS_SET                (1<<TEST_ID)
#define LUMIKEY_ID_KEY_IS_SET                (1<<LUMIKEY_ID)

/*
**    Symmetric Key Type Memory Map
**  MUST be Packed and Mod4
*/
typedef struct                    // Symmetric Key Format
{
    u16   nVER;                    // Version
    u16   nKEYMODE;                // Key mode determines the CrptoScheme and KeySize
    u8    nKCV[4];                // Key Check Value
    u256 nKEY;                    // Key
}   SymKeyFormatType;

typedef struct                    // Asymmetric Key Format
{
    u16   nVER;                    // Version
    u16   nKEYMODE;                // Key mode determines the CrptoScheme and KeySize
    u8    nKCV[4];                // Key Check Value (SHA-256 HASH)
    u2048 nKEY;                    // Key
}   AsymKeyFormatType;

/*
**    DUKPT Counter Mode
*/
typedef enum {
    DUKPT_RUN     = 0x00,        // normal incrementing counter mode
    DUKPT_HALT,                // do not advance counter
    DUKPT_RESET_TC0,        // reset transaction counter #0
    DUKPT_RESET_TC1,        // reset transaction counter #1
    DUKPT_END
}   DUKPT_Counter_Mode;


/*
**    Encryption Modes
*/
typedef enum
{
    NO_MODE        = 0x00,
    AES_MODE    = 0x01,
    TDES_MODE   = 0x02,
    MODE_LAST   = 0x03
}   EncryptionModeType;

typedef enum
{
    KMT_MODE_NONE            = 0,        // Invalid Key Mode.
    KMT_AES_256_CBC            = 1,        // AES-256 using Cipher Block Chaining (CBC) cipher mode
    KMT_AES_128_CBC            = 2,        // AES-128 using Cipher Block Chaining (CBC) cipher mode
    KMT_TDES_ABA_ECB        = 3,        // Two-Key TDES (ABA Key Format) using Electronic Cookbook (ECB) cipher mode
    KMT_TDES_ABA_CBC        = 4,        // Two-Key TDES (ABA Key Format) using Cipher Block Chaining (CBC) cipher mode
    KMT_TDES_ABC_ECB        = 5,        // Three-Key TDES (ABC Key Format) using Electronic Cookbook (ECB) cipher mode
    KMT_TDES_ABC_CBC        = 6,        // Three-Key TDES (ABC Key Format) using Cipher Block Chaining (CBC) cipher mode
    KMT_RSA_2048_v15        = 0x1000,    // RSA-2048 (version 1.5)
    KMT_RSA_2048_v21        = 0x1001,    // RSA-2048 (version 2.1)
    KMT_DUKPT_IPEK_128        = 0x1002,    // DUKPT - Initial Pin Encryption Key
    KMT_DUKPT_KSN_64        = 0x1003,    // DUKPT - Key Serial Number
}   KeyMode_Type;

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

