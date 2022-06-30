#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "lumi_stdint.h"
#include "Platform.h"

#include "polarssl/lumi_config.h"

#include "polarssl/aes.h"
#include "polarssl/bignum.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/des.h"
#include "polarssl/md.h"
#include "polarssl/rsa.h"
#include "polarssl/sha1.h"
#include "polarssl/sha256.h"
#include "polarssl/sha512.h"
#include "polarssl/ecdsa.h"
#include "polarssl/ecp.h"
#include "polarssl/x509.h"

typedef enum
{
	AES_ECB,
	AES_CBC,
	AES_CTR
} AES_BLOCK_Modes_Type;

/* Preprocessor directives */
/*
**	Supported Modes of Operation
*/
typedef enum
{
	DES_ECB,
	DES_CBC,
	TDES_ECB,
	TDES_CBC
} DES_Mode_Type;

/*
** NULL Encryption
*/

#define NULL_NULL	99

typedef enum 
{
	SHA1_MODE,
	SHA256_MODE,
	SHA224_MODE,	
	SHA384_MODE,
	SHA512_MODE,
}HMAC_SHA_MODE_Type;

/*
**  Secure Keys stored in SDRAM Section {KEYS}
** KEYS SEGMENT is 2048 Bytes
*/
#define NUM_SRAM_KEYS	16		    // 512B allocation in "KEYS" segment


#define EXPONENT 65537

//#define POLARSSL_MPI_MAX_BITS                              ( 8 * POLARSSL_MPI_MAX_SIZE )    /**< Maximum number of bits for usable MPIs. */

/*
 * Define the base integer type, architecture-wise
 */
//typedef int32_t t_sint;
//typedef u32 t_uint;
//typedef u64 t_udbl;
 
/*
 * PKCS#1 constants
 */
#define SIG_RSA_RAW     0
#define SIG_RSA_MD2     2
#define SIG_RSA_MD4     3
#define SIG_RSA_MD5     4
#define SIG_RSA_SHA1    5
#define SIG_RSA_SHA256 11
#define SIG_RSA_SHA384 12
#define SIG_RSA_SHA512 13
#define SIG_RSA_SHA224 14
#define SIG_RSA_SHA512_224 15
#define SIG_RSA_SHA512_256 16


/*
* ECDSA constants
*/
//SANG -we might have to consolidate these constants
#define SIG_ECDSA_SHA1    5
#define SIG_ECDSA_SHA256 11
#define SIG_ECDSA_SHA384 12
#define SIG_ECDSA_SHA512 13
#define SIG_ECDSA_SHA224 14
#define SIG_ECDSA_SHA512_224 15
#define SIG_ECDSA_SHA512_256 16


#pragma pack(1)
typedef struct
{
	u64 CNTR   : 21;	// right-most 21b Transaction Counter
	u64	DID    : 43;	
//	u64 GID    : 8;
//	u64 CID    : 8;
//	u64 INNLSB : 8;	
	u16	INNMSB;			// Issuer Identifier Number
} KSNType;
#pragma pack()

typedef enum
{
	_LE_ = 0,
	_BE_ = 1
} EndianType;

typedef enum
{
	TC0 = 0,
	TC1 = 1,
	TCLAST
} TransactionCounterType;


/*
**	Session Key update Mode
*/
//#define MAX_KCV_SIZE	16

