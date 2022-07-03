#pragma once

/**
 * \def POLARSSL_HAVE_LONGLONG
 *
 * The compiler supports the 'long long' type.
 * (Only used on 32-bit platforms)
 */
//#define POLARSSL_HAVE_LONGLONG

/**
 * \def POLARSSL_HAVE_ASM
 *
 * The compiler has support for asm()
 *
 * Uncomment to enable the use of assembly code.
 *
 * Requires support for asm() in compiler.
 *
 * Used in:
 *      library/timing.c
 *      library/padlock.c
 *      include/polarssl/bn_mul.h
 *
 */
//#define POLARSSL_HAVE_ASM

/**
 * \def POLARSSL_GENPRIME
 *
 * Requires: POLARSSL_BIGNUM_C, POLARSSL_RSA_C
 *
 * Enable the RSA prime-number generation code.
 */
#define POLARSSL_GENPRIME

/**
 * \def POLARSSL_PKCS1_V21
 *
 * Requires: POLARSSL_MD_C, POLARSSL_RSA_C
 *
 * Enable support for PKCS#1 v2.1 encoding.
 * This enables support for RSAES-OAEP and RSASSA-PSS operations.
 */
#define POLARSSL_PKCS1_V21

/**
 * \def POLARSSL_RSA_NO_CRT
 *
 * Do not use the Chinese Remainder Theorem for the RSA private operation.
 *
 * Uncomment this macro to disable the use of CRT in RSA.
 *
 */
#define POLARSSL_RSA_NO_CRT
 // SPC uncomment

/**
 * \def POLARSSL_SELF_TEST
 *
 * Enable the checkup functions (*_self_test).
 */
#define POLARSSL_SELF_TEST

/**
 * \def POLARSSL_BIGNUM_C
 *
 * Enable the multi-precision integer library.
 *
 * Module:  library/bignum.c
 * Caller:  library/dhm.c
 *          library/rsa.c
 *          library/ssl_tls.c
 *          library/x509parse.c
 *
 * This module is required for RSA and DHM support.
 */
#define POLARSSL_BIGNUM_C

/**
 * \def POLARSSL_MD_C
 *
 * Enable the generic message digest layer.
 *
 * Module:  library/md.c
 * Caller:
 *
 * Uncomment to enable generic message digest wrappers.
 */
#define POLARSSL_MD_C

/**
 * \def POLARSSL_PKCS12_C
 *
 * Enable PKCS#12 PBE functions
 * Adds algorithms for parsing PKCS#8 encrypted private keys
 *
 * Module:  library/pkcs12.c
 * Caller:  library/x509parse.c
 *
 * Requires: POLARSSL_ASN1_PARSE_C, POLARSSL_CIPHER_C, POLARSSL_MD_C
 * Can use:  POLARSSL_ARC4_C
 *
 * This module enables PKCS#12 functions.
 */
#define POLARSSL_PKCS12_C

/**
 * \def POLARSSL_RSA_C
 *
 * Enable the RSA public-key cryptosystem.
 *
 * Module:  library/rsa.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *          library/x509.c
 *
 * Requires: POLARSSL_BIGNUM_C
 *
 * This module is required for SSL/TLS and MD5-signed certificates.
 */
#define POLARSSL_RSA_C

/**
 * \def POLARSSL_SHA1_C
 *
 * Enable the SHA1 cryptographic hash algorithm.
 *
 * Module:  library/sha1.c
 * Caller:  library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *          library/x509parse.c
 *
 * This module is required for SSL/TLS and SHA1-signed certificates.
 */
#define POLARSSL_SHA1_C


