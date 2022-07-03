#pragma once
#include <cstddef>
#include "lumi_stdint.h"

/*
 * Sequence Values of Note
 */
#define _V100_KEY_CACHE_INVALID_SEQUENCE 0xFFFFFFFF
#define _V100_KEY_CACHE_PARTITION_SEQUENCE 0x80000000
/*
 * Key Cache Structure
 */
typedef struct
{
    uint32_t  flags;    // ENC_KEY_CACHE bits, defined above...
    uint32_t  seq;        // sequence of use, used to determine ephemeral key expiry
    size_t    length;    // length of the stored key in bytes
    uint8_t * key_data;    // pointer to the stored bytes
} _V100_KEY_CACHE_ROW;

/*
* key cache attributes in _V100_KEY_CACHE_ROW flags field
*/
#define ENC_KEY_CACHE_LOCKED         1  // does a file exist on flash for this key?
#define ENC_KEY_CACHE_WRITEABLE      2  // can this be directly set?
#define ENC_KEY_CACHE_PUBLIC         4  // is this a public key / csr or cert?  i.e. can this be queried by client?
#define ENC_KEY_CACHE_PRIVATE        8  // is this a private asymmetric key?
#define ENC_KEY_CACHE_SECRET        16  // is this a secret symmetric key?
#define ENC_KEY_CACHE_EPHEMERAL     32  // is this a secret ephemeral key?
#define ENC_KEY_CACHE_IV            64  // is this an initialization vector?
#define ENC_KEY_CACHE_NONCE        128  // is this a nonce?
#define ENC_KEY_CACHE_KAS_PARAM    256  // is this a Key Agreement Scheme parameter?
#define ENC_KEY_CACHE_AUTH_TOKEN   512    // is this an authentication token?
#define ENC_KEY_CACHE_LOCKABLE      1024    // Is it possible to lock this key?

/*
** Global Lock
*/

#define ENC_GLOBAL_LOCK_ON        897300
#define ENC_GLOBAL_LOCK_OFF        122578


/*
**  Extended Key Area Keynames and Parameters
*/
typedef enum
{
    KT_FIPS_INVALID = -1,
    KT_FIPS_NUM_CERT_CONTEXTS = 5,
    KT_FIPS_CERT_CONTEXT_WIDTH = 40,
    KT_FIPS_EPHEMERAL_WIDTH = 6,
    KT_FIPS_CERT_CRT_WIDTH = 7,

    // ON-DEVICE_CACHE
    KT_FIPS_EKEY_MASTER = 0,
    KT_FIPS_MKEY_MASTER = 1,
    KT_FIPS_DEV_PRIVATE_SIGN = 2,
    KT_FIPS_DEV_PRIVATE_ENCRYPT = 3,
    KT_FIPS_DEV_PUBLIC_SIGN = 4,
    KT_FIPS_DEV_PUBLIC_ENCRYPT = 5,
    KT_FIPS_DEV_CSR_SIGN = 6,
    KT_FIPS_DEV_CSR_ENCRYPT = 7,

    // HOST CACHE
    KT_FIPS_HOST_PRIVATE_SIGN = 8,
    KT_FIPS_HOST_PRIVATE_ENCRYPT = 9,

    KT_FIPS_FIRMWARE_AUTH_TOKEN = 10,
    KT_FIPS_FKEY_MASTER = 11,

    // GLOBAL LOCK
    KT_FIPS_GLOBAL_LOCK = 12,

    KT_FIPS_CERT_CONTEXT_0_BASE = 32,
    KT_FIPS_DEV_ROOT_CA_0 = 32,
    KT_FIPS_HOST_ROOT_CA_0 = 33,
    KT_FIPS_DEV_CRT_0_ENCRYPT = 34,
    KT_FIPS_DEV_CRT_0_SIGN = 35,
    KT_FIPS_HOST_CRT_0_ENCRYPT = 36,
    KT_FIPS_HOST_CRT_0_SIGN = 37,
    KT_FIPS_MAX_CERT_CONTEXT_0 = 38,

    KT_FIPS_NONCE_H2D_0 = 39,
    KT_FIPS_NONCE_D2H_0 = 40,
    KT_FIPS_IV_H2D_0 = 41,
    KT_FIPS_IV_D2H_0 = 42,

    KT_FIPS_CHALLENGE_0 = 43,
    KT_FIPS_RESPONSE_0 = 44,
    KT_FIPS_SP800_56B_CU_0 = 45,
    KT_FIPS_SP800_56B_CV_MACTAGV_0 = 46,
    KT_FIPS_SP800_56B_MACTAGU_0 = 47,

    KT_FIPS_EPHEMERAL_0_BASE = 66,
    KT_FIPS_EKEY_AES_128_GCM_H2D_0 = 66,
    KT_FIPS_EKEY_AES_128_GCM_D2H_0 = 67,
    KT_FIPS_EKEY_AES_128_CTR_H2D_0 = 68,
    KT_FIPS_EKEY_AES_128_CTR_D2H_0 = 69,
    KT_FIPS_MKEY_H2D_0 = 70,
    KT_FIPS_MKEY_D2H_0 = 71,

    KT_FIPS_CERT_CONTEXT_1_BASE = 72,
    KT_FIPS_DEV_ROOT_CA_1 = 72,
    KT_FIPS_HOST_ROOT_CA_1 = 73,
    KT_FIPS_DEV_CRT_1_ENCRYPT = 74,
    KT_FIPS_DEV_CRT_1_SIGN = 75,
    KT_FIPS_HOST_CRT_1_ENCRYPT = 76,
    KT_FIPS_HOST_CRT_1_SIGN = 77,
    KT_FIPS_MAX_CERT_CONTEXT_1 = 78,

    KT_FIPS_NONCE_H2D_1 = 79,
    KT_FIPS_NONCE_D2H_1 = 80,
    KT_FIPS_IV_H2D_1 = 81,
    KT_FIPS_IV_D2H_1 = 82,

    KT_FIPS_CHALLENGE_1 =83,
    KT_FIPS_RESPONSE_1 = 84,
    KT_FIPS_SP800_56B_CU_1 = 85,
    KT_FIPS_SP800_56B_CV_MACTAGV_1 = 86,
    KT_FIPS_SP800_56B_MACTAGU_1 = 87,

    KT_FIPS_EPHEMERAL_1_BASE = 106,
    KT_FIPS_EKEY_AES_128_GCM_H2D_1 = 106,
    KT_FIPS_EKEY_AES_128_GCM_D2H_1 = 107,
    KT_FIPS_EKEY_AES_128_CTR_H2D_1 = 108,
    KT_FIPS_EKEY_AES_128_CTR_D2H_1 = 109,
    KT_FIPS_MKEY_H2D_1 = 110,
    KT_FIPS_MKEY_D2H_1 = 111,

    KT_FIPS_CERT_CONTEXT_2_BASE = 112,
    KT_FIPS_DEV_ROOT_CA_2 = 112,
    KT_FIPS_HOST_ROOT_CA_2 = 113,
    KT_FIPS_DEV_CRT_2_ENCRYPT = 114,
    KT_FIPS_DEV_CRT_2_SIGN = 115,
    KT_FIPS_HOST_CRT_2_ENCRYPT = 116,
    KT_FIPS_HOST_CRT_2_SIGN = 117,
    KT_FIPS_MAX_CERT_CONTEXT_2 = 118,

    KT_FIPS_NONCE_H2D_2 = 119,
    KT_FIPS_NONCE_D2H_2 = 120,
    KT_FIPS_IV_H2D_2 = 121,
    KT_FIPS_IV_D2H_2 = 122,

    KT_FIPS_CHALLENGE_2 = 123,
    KT_FIPS_RESPONSE_2 = 124,
    KT_FIPS_SP800_56B_CU_2 = 125,
    KT_FIPS_SP800_56B_CV_MACTAGV_2 = 126,
    KT_FIPS_SP800_56B_MACTAGU_2 = 127,

    KT_FIPS_EPHEMERAL_2_BASE = 146,
    KT_FIPS_EKEY_AES_128_GCM_H2D_2 = 146,
    KT_FIPS_EKEY_AES_128_GCM_D2H_2 = 147,
    KT_FIPS_EKEY_AES_128_CTR_H2D_2 = 148,
    KT_FIPS_EKEY_AES_128_CTR_D2H_2 = 149,
    KT_FIPS_MKEY_H2D_2 = 150,
    KT_FIPS_MKEY_D2H_2 = 151,

    KT_FIPS_CERT_CONTEXT_3_BASE = 152,
    KT_FIPS_DEV_ROOT_CA_3 = 152,
    KT_FIPS_HOST_ROOT_CA_3 = 153,
    KT_FIPS_DEV_CRT_3_ENCRYPT = 154,
    KT_FIPS_DEV_CRT_3_SIGN = 155,
    KT_FIPS_HOST_CRT_3_ENCRYPT = 156,
    KT_FIPS_HOST_CRT_3_SIGN = 157,
    KT_FIPS_MAX_CERT_CONTEXT_3 = 158,

    KT_FIPS_NONCE_H2D_3 = 159,
    KT_FIPS_NONCE_D2H_3 = 160,
    KT_FIPS_IV_H2D_3 = 161,
    KT_FIPS_IV_D2H_3 = 162,

    KT_FIPS_CHALLENGE_3 = 163,
    KT_FIPS_RESPONSE_3 = 164,
    KT_FIPS_SP800_56B_CU_3 = 165,
    KT_FIPS_SP800_56B_CV_MACTAGV_3 = 166,
    KT_FIPS_SP800_56B_MACTAGU_3 = 167,

    KT_FIPS_EPHEMERAL_3_BASE = 186,
    KT_FIPS_EKEY_AES_128_GCM_H2D_3 = 186,
    KT_FIPS_EKEY_AES_128_GCM_D2H_3 = 187,
    KT_FIPS_EKEY_AES_128_CTR_H2D_3 = 188,
    KT_FIPS_EKEY_AES_128_CTR_D2H_3 = 189,
    KT_FIPS_MKEY_H2D_3 = 190,
    KT_FIPS_MKEY_D2H_3 = 191,

    KT_FIPS_CERT_CONTEXT_4_BASE = 192,
    KT_FIPS_DEV_ROOT_CA_4 = 192,
    KT_FIPS_HOST_ROOT_CA_4 = 193,
    KT_FIPS_DEV_CRT_4_ENCRYPT = 194,
    KT_FIPS_DEV_CRT_4_SIGN = 195,
    KT_FIPS_HOST_CRT_4_ENCRYPT = 196,
    KT_FIPS_HOST_CRT_4_SIGN = 197,
    KT_FIPS_MAX_CERT_CONTEXT_4 = 198,

    KT_FIPS_NONCE_H2D_4 = 199,
    KT_FIPS_NONCE_D2H_4 = 200,
    KT_FIPS_IV_H2D_4 = 201,
    KT_FIPS_IV_D2H_4 = 202,

    KT_FIPS_CHALLENGE_4 = 203,
    KT_FIPS_RESPONSE_4 = 204,
    KT_FIPS_SP800_56B_CU_4 = 205,
    KT_FIPS_SP800_56B_CV_MACTAGV_4 = 206,
    KT_FIPS_SP800_56B_MACTAGU_4 = 207,

    KT_FIPS_EPHEMERAL_4_BASE = 226,
    KT_FIPS_EKEY_AES_128_GCM_H2D_4 = 226,
    KT_FIPS_EKEY_AES_128_GCM_D2H_4 = 227,
    KT_FIPS_EKEY_AES_128_CTR_H2D_4 = 228,
    KT_FIPS_EKEY_AES_128_CTR_D2H_4 = 229,
    KT_FIPS_MKEY_H2D_4 = 230,
    KT_FIPS_MKEY_D2H_4 = 231,
    KT_FIPS_KEYMAP_END = 232,
} _V100_ENC_FIPS_KEY_TYPE;

extern bool _V100_key_cache_update(_V100_ENC_FIPS_KEY_TYPE slot, const uint8_t * key_data, size_t length);
extern bool _V100_key_cache_append(_V100_ENC_FIPS_KEY_TYPE slot, const uint8_t * key_data, size_t length);
extern bool _V100_key_cache_get_copy(_V100_ENC_FIPS_KEY_TYPE slot, uint8_t ** key_data, size_t & length);
extern const uint8_t * _V100_key_cache_get_ptr(_V100_ENC_FIPS_KEY_TYPE slot);
extern const char * _V100_key_cache_get_char_ptr(_V100_ENC_FIPS_KEY_TYPE slot);
extern size_t _V100_key_cache_get_length(_V100_ENC_FIPS_KEY_TYPE slot);
extern uint32_t _V100_key_cache_get_seq(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_increment_seq(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_set_seq(_V100_ENC_FIPS_KEY_TYPE slot, uint32_t seq);
extern void _V100_key_cache_clear_slot(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_is_CSR_slot(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_is_root_CA_slot(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_is_cert_slot(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_is_private_key_slot(_V100_ENC_FIPS_KEY_TYPE slot);
extern bool _V100_key_cache_is_writeable_slot(_V100_ENC_FIPS_KEY_TYPE slot);
extern _V100_KEY_CACHE_ROW* _v100_get_key_cache();

