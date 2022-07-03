/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_rsa.cpp 22877 2014-03-03 03:38:32Z spcorcoran $
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
/*
**    Includes
*/
#include <assert.h>
#include <time.h>
//#include <sys/time.h>

#include "ICypher.h"
#include "CypherTypes.h"
#include "c_rsa.h"
#include "c_sha1.h"
#include "c_sha256.h"
#include "c_envelope.h"
#include "lumi_random.h"
#include "Auto_Crypto_Objects.h"

#define MAX_KEY_SIZE 2048

static char rid[] = "$Rev: 23052 $";


/******************************************************************************
**
******************************************************************************/
static md_type_t PKCS_1_to_PolarSSL_hash_id(int hash_id)
{
    switch (hash_id)
    {
        case SIG_RSA_SHA1:
            return POLARSSL_MD_SHA1;    //break;//SANG
        case SIG_RSA_SHA224:
            return POLARSSL_MD_SHA224;  //break;//SANG
        case SIG_RSA_SHA256:
            return POLARSSL_MD_SHA256;  //break;//SANG
        case SIG_RSA_SHA384:
            return POLARSSL_MD_SHA384;  //break;//SANG
        case SIG_RSA_SHA512:
            return POLARSSL_MD_SHA512;  //break;//SANG
    }
    return md_type_t(-1);
}

/******************************************************************************
**
******************************************************************************/
oRSA::oRSA()
{
    memset(&ctx, 0, sizeof(ctx));
    Init();// IF you remove this make sure client API calls Init..
    SetRandTestMode(false, NULL, 0);
}

/******************************************************************************
**
******************************************************************************/
oRSA::~oRSA()
{
    Clear();
}

/******************************************************************************
** Revision
******************************************************************************/
u32 oRSA::Get_Revision(void)
{
    int rev;
       sscanf(&rid[6], "%d", &rev);
    return (u32)rev;
}

/******************************************************************************
** Initialize an RSA context
******************************************************************************/
bool oRSA::Init(int padding, int hash_id)
{
    return SetMode(padding, hash_id);
}

/******************************************************************************
** Initialize an RSA context
******************************************************************************/
void oRSA::Clear()
{
    rsa_free(&ctx);
    SetRandTestMode(false, NULL, 0);
    //bUseCRT = true;
}

/******************************************************************************
** Initialize Test Mode, This permits disabling RNG and using known SEEDS to
**    generate known answer results with RSA encryption.  Test mode should be
**    off for all security related deployments and ONLY enabled for Test Vector
**    Qualification Tests
******************************************************************************/
bool oRSA::SetRandTestMode(bool bMode, u8 *pSeed, int nSeedLen)
{
    if (bMode)
    {
        lumi_random_init(pSeed, nSeedLen);
    }
    else
    {
        lumi_random_init(NULL, 0);
    }
    return true;
}

/******************************************************************************
** Initialize an RSA context
******************************************************************************/
bool oRSA::SetMode(int Padding, int Hash)
{
    switch (Padding)
    {
        case RSA_PKCS_V15:
        case RSA_PKCS_V21:
            break;
        default:
            return false;
    }

    switch (Hash)
    {
        case SIG_RSA_SHA1:
        case SIG_RSA_SHA224:
        case SIG_RSA_SHA256:
        case SIG_RSA_SHA384:
        case SIG_RSA_SHA512:
            break;
        default:
            return false;
    }
    rsa_free(&ctx);
    rsa_init(&ctx, Padding, PKCS_1_to_PolarSSL_hash_id(Hash));
    return true;
}

/******************************************************************************
** Add the message padding, then do an RSA operation
******************************************************************************/
bool oRSA::Encrypt(void *p_rng, int mode, size_t ilen, u8 *input, u8 *output)
{
    int rc = -1;
    switch (ctx.padding)
    {
        case RSA_PKCS_V15:
            rc = rsa_rsaes_pkcs1_v15_encrypt(&ctx, lumi_random, p_rng, mode, ilen, input, output);
            break;
        case RSA_PKCS_V21:
            rc = rsa_rsaes_oaep_encrypt(&ctx, lumi_random, p_rng, mode, NULL, 0, ilen, input, output);
            break;
    }
    return rc == 0;
}

/******************************************************************************
** Do an RSA operation, then remove the message padding
******************************************************************************/
bool oRSA::Decrypt(int mode, size_t *olen, u8 *input, u8 *output, size_t output_max_len)
{
    int rc = -1;
    switch (ctx.padding)
    {
        case RSA_PKCS_V15:
            rc = rsa_rsaes_pkcs1_v15_decrypt(&ctx, lumi_random, NULL, mode, olen, input, output, output_max_len);
            break;
        case RSA_PKCS_V21:
            rc = rsa_rsaes_oaep_decrypt(&ctx, lumi_random, NULL, mode, NULL, 0, olen, input, output, output_max_len);
            break;
    }
    return rc == 0;
}

/******************************************************************************
** Do an RSA operation to sign the message digest
******************************************************************************/
bool oRSA::Sign(void *p_rng, int mode, int hash_id, u32 hashlen, u8 *hash, u8 *sig)
{
    int rc = -1;
    switch (ctx.padding)
    {
        case RSA_PKCS_V15:
            rc = rsa_rsassa_pkcs1_v15_sign(&ctx, lumi_random, p_rng, mode, PKCS_1_to_PolarSSL_hash_id(hash_id), hashlen, hash, sig);
            break;
        case RSA_PKCS_V21:
            rc = rsa_rsassa_pss_sign(&ctx, lumi_random, p_rng, mode, PKCS_1_to_PolarSSL_hash_id(hash_id), hashlen, hash, sig);
            break;
    }
    return rc == 0;
}

/******************************************************************************
** Calculate message digest for give message and then do an RSA operation to sign the message digest
******************************************************************************/
bool oRSA::CalHashAndSign(void *p_rng, int mode, int hash_id, u32 ilen, const u8 *input, u8 *sig)
{
    AutoHeapBuffer Auto_hash(POLARSSL_MD_MAX_SIZE);
    if (Auto_hash.u8Ptr() == NULL)
        return false;

    u32 hashlen;
    switch (hash_id)
    {
        #if 0
        // THOU SHALT NOT: generate signatures using SHA1 as the hash!
        case SIG_RSA_SHA1:
        {
            ISHA1 * pSHA1 = ICypher::GetInstance()->GetSHA1();
            pSHA1->Hash(input, ilen, Auto_hash.u8Ptr());
            hashlen = 160/8;
            break;
        }
        #endif
        case SIG_RSA_SHA224:
        {
            ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
            pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is224*/);
            hashlen = 224/8;
            break;
        }
        case SIG_RSA_SHA256:
        {
            ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
            pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is224*/);
            hashlen = 256/8;
            break;
        }
        case SIG_RSA_SHA384:
        {
            ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
            pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is384*/);
            hashlen = 384/8;
            break;
        }
        case SIG_RSA_SHA512:
        {
            ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
            pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is384*/);
            hashlen = 512/8;
            break;
        }
        default:
            return false;
    }

    bool bRC = Sign(p_rng, mode, hash_id, hashlen, Auto_hash.u8Ptr(), sig);
    return bRC;
}

/******************************************************************************
** Do an RSA operation and check the message digest
******************************************************************************/
bool oRSA::Verify(int mode, int hash_id, u32 hashlen, const u8 *hash, const u8 *sig)
{
    int rc = -1;
    switch (ctx.padding)
    {
        case RSA_PKCS_V15:
            rc = rsa_rsassa_pkcs1_v15_verify(&ctx, lumi_random, NULL, mode, PKCS_1_to_PolarSSL_hash_id(hash_id), hashlen, hash, sig);
            break;
        case RSA_PKCS_V21:
            rc = rsa_rsassa_pss_verify(&ctx, lumi_random, NULL, mode, PKCS_1_to_PolarSSL_hash_id(hash_id), hashlen, hash, sig);
            break;
    }
    return rc == 0;
}

bool oRSA::CalHashAndVerify(int mode, int hash_id, u32 ilen, u8 *input, u8 *sig)
{
    AutoHeapBuffer Auto_hash(POLARSSL_MD_MAX_SIZE);
    if (Auto_hash.u8Ptr() == NULL)
        return false;

    u32 hashlen;
    switch (hash_id)
    {
        case SIG_RSA_SHA1:
        {
            ISHA1 * pSHA1 = ICypher::GetInstance()->GetSHA1();
            pSHA1->Hash(input, ilen, Auto_hash.u8Ptr());
            hashlen = 160/8;
            break;
        }
        case SIG_RSA_SHA224:
        {
            ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
            pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is224*/);
            hashlen = 224/8;
            break;
        }
        case SIG_RSA_SHA256:
        {
            ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
            pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is224*/);
            hashlen = 256/8;
            break;
        }
        case SIG_RSA_SHA384:
        {
            ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
            pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is384*/);
            hashlen = 384/8;
            break;
        }
        case SIG_RSA_SHA512:
        {
            ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
            pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is384*/);
            hashlen = 512/8;
            break;
        }
        default:
            return false;
    }

    bool bRC = Verify(mode, hash_id, hashlen, Auto_hash.u8Ptr(), sig);
    return bRC;
}

/******************************************************************************
** Generate an RSA keypair
** NOTE:      NIST only permits exponents > 2^16
**             {3,5,17,257,65537} smaller is faster  65537 is default
******************************************************************************/
bool oRSA::Generate_Keypair(void *p_rng, u32 nbits, int exponent)
{
    int rc = -1;

    Clear();
    Init();

    SetRandTestMode(p_rng ? true : false, reinterpret_cast<u8*>(p_rng), nbits / 8);
    rc = rsa_gen_key(&ctx, lumi_random, NULL, nbits, exponent);

    return rc == 0;
}

/******************************************************************************
** Return Public Key
******************************************************************************/
bool oRSA::GetPublicKey(u8 *pKey, u32 *rSize)
{
    size_t nBytes = ctx.N.n * sizeof(t_uint);                    // number of bytes

    if(mpi_write_binary(&(ctx.N), pKey, nBytes) !=0) return false;        // export
    *rSize = (u32)nBytes;

    return true;
}

/******************************************************************************
** Return Private Key
******************************************************************************/
bool oRSA::GetPrivateKey(u8 *pKey, u32 *rSize)
{
    size_t nBytes = ctx.D.n * sizeof(t_uint);                    // number of bytes

    if (mpi_write_binary(&ctx.D, pKey, nBytes) != 0) return false;    // export
    *rSize = (u32)nBytes;

    return true;
}

/******************************************************************************
** Return Primes from Context
******************************************************************************/
bool oRSA::GetPrimes(u8 *pP, u8 *pQ, u32 *Exp, u32 nSize)
{
    size_t nBytes = ctx.P.n * sizeof(t_uint);            // number of bytes
    if (nSize != nBytes)
        return false;

    if (mpi_write_binary(&ctx.P, pP, nBytes) != 0) return false;        // export
    if (mpi_write_binary(&ctx.Q, pQ, nBytes) != 0) return false;        // export
    if (mpi_write_binary(&ctx.E, (u8*)Exp, sizeof(u32)) != 0) return false;        // no need

    return true;
}

/******************************************************************************
**  Load the Context
**    Must have
******************************************************************************/
bool oRSA::SetContextKeyPair(const u8 *pN, const u8 *pD, u32 rSize, u32 Exponent)
{
    int padding = ctx.padding;
    int hash_id = ctx.hash_id;
    rsa_free(&ctx);
    rsa_init(&ctx, padding, hash_id);

    if (mpi_lset(&(ctx.E), Exponent)         != 0 ||
        mpi_read_binary(&(ctx.N), pN, rSize) != 0 ||
        mpi_read_binary(&(ctx.D), pD, rSize) != 0)
        return false;

    ctx.len = (mpi_msb(&ctx.N) + 7) >> 3;

    int rc = -1;
    rc = rsa_check_pubkey(&ctx);
    if (rc != 0)
         return false;
#if 0
    // we can't do this because P and Q are not set - just N and D...
    rc = rsa_check_privkey(&ctx);
    if (rc != 0)
        return false;
#endif

    return true;
}

/******************************************************************************
**  Load the Context
**    Must have E, P, and Q
******************************************************************************/
bool oRSA::SetContext(const u8 *pP, const u8 *pQ, u32 Exponent, u32 rSize)
{
    int padding = ctx.padding;
    int hash_id = ctx.hash_id;
    rsa_free(&ctx);
    rsa_init(&ctx, padding, hash_id);

    Auto_mpi P1, Q1, H, G, G2, L1, L2;
    u32 eTime = 0;
    u32 stime = ICypher::GetInstance()->Timer_Start();

    if (mpi_lset(&ctx.E, Exponent) != 0)
        return false;
    if (mpi_read_binary(&ctx.P, pP, rSize) != 0)
        return false;
    if (mpi_read_binary(&ctx.Q, pQ, rSize) != 0)
        return false;

    if (mpi_mul_mpi(&ctx.N, &ctx.P, &ctx.Q) != 0)
        return false;
    if (mpi_sub_int(P1.Ptr(), &ctx.P, 1) != 0)
        return false;
    if (mpi_sub_int(Q1.Ptr(), &ctx.Q, 1) != 0)
        return false;
    if (mpi_mul_mpi(H.Ptr(), P1.Ptr(), Q1.Ptr()) != 0)
        return false;

    if (mpi_gcd(G2.Ptr(), P1.Ptr(), Q1.Ptr()) != 0)
        return false;
    if (mpi_div_mpi(L1.Ptr(), L2.Ptr(), H.Ptr(), G2.Ptr()) != 0)
        return false;
    if (mpi_inv_mod(&(ctx.D), &(ctx.E), L1.Ptr()) != 0)
        return false;
    //if (mpi_inv_mod(&(ctx.D), &(ctx.E), H.Ptr()) != 0)
    //    return false;

    if (mpi_mod_mpi(&(ctx.DP), &(ctx.D), P1.Ptr()) != 0)
        return false;
    if (mpi_mod_mpi(&(ctx.DQ), &(ctx.D), Q1.Ptr()) != 0)
        return false;
    if (mpi_inv_mod(&(ctx.QP), &(ctx.Q), &(ctx.P)) != 0)
        return false;

    ctx.len = (mpi_msb(&ctx.N) + 7) >> 3;

    int rc = -1;
    rc = rsa_check_pubkey(&ctx);
    if (rc != 0)
        return false;
    rc = rsa_check_privkey(&ctx);
    if (rc != 0)
         return false;

    eTime = 0;
    eTime += ICypher::GetInstance()->Timer_Stop(stime);
    return true;
}

/******************************************************************************
**  Load a Clean Context
**    Must have Public Key ONLY
******************************************************************************/
bool oRSA::SetContextPublic(const u8 *pKey, u32 nBytes, u32 Exponent)
{
    int padding = ctx.padding;
    int hash_id = ctx.hash_id;
    rsa_free(&ctx);
    rsa_init(&ctx, padding, hash_id);

    u32 eTime = 0;
    u32 stime = ICypher::GetInstance()->Timer_Start();

    /*
    **    Fill in Context
    */
    if(mpi_lset(&(ctx.E), Exponent) != 0 ) return false;
    if (mpi_read_binary(&(ctx.N), pKey, nBytes) != 0) return false;

    ctx.len = (mpi_msb(&(ctx.N)) + 7) >> 3;

    int rc = -1;
    rc = rsa_check_pubkey(&ctx);
    if (rc != 0)
        return false;

    eTime = 0;
    eTime += ICypher::GetInstance()->Timer_Stop(stime);

   return true;
}

/******************************************************************************
**  Load a Clean Context from MAXQ Secure Memory
**    Must Public+Private Keys , P and Q and EXP
******************************************************************************/
bool oRSA::Restore_Device_Context(void)
{
    return false;
}



/******************************************************************************
** get the RSA public key in PEM format
******************************************************************************/
bool oRSA::GetPublicKeyPEM(char *pKey, u32 *rSize)
{
    Auto_pk_context auto_pkctx;
    pk_context * pkctx = auto_pkctx.Ptr();
    if (!pkctx)
        return false;
    pk_init(pkctx);
    pkctx->pk_info = pk_info_from_type(POLARSSL_PK_RSA);
    pkctx->pk_ctx = &ctx;
    int rc = pk_write_pubkey_pem(pkctx, reinterpret_cast<unsigned char *>(pKey), *rSize);
    if (rc == 0)
        *rSize = static_cast<u32>(strlen(pKey) + 1);
    else
        rSize = 0;
    pkctx->pk_ctx = NULL;   // avoid horrible repercussions!
    pkctx->pk_info = NULL;
    return (rc == 0);
}

/******************************************************************************
** get the RSA keypair in PEM format
******************************************************************************/
bool oRSA::GetPrivateKeyPEM(char *pKey, u32 *rSize)
{
    Auto_pk_context auto_pkctx;
    pk_context * pkctx = auto_pkctx.Ptr();
    if (!pkctx)
        return false;
    pk_init(pkctx);
    pkctx->pk_info = pk_info_from_type(POLARSSL_PK_RSA);
    pkctx->pk_ctx = &ctx;
    int rc = pk_write_key_pem(pkctx, reinterpret_cast<unsigned char*>(pKey), *rSize);
    if (rc == 0)
        *rSize = static_cast<u32>(strlen(pKey) + 1);
    else
        *rSize = 0;
    pkctx->pk_ctx = NULL;   // avoid horrible repercussions!
    pkctx->pk_info = NULL;
    return (rc == 0);
}

/******************************************************************************
** set the RSA keypair via PEM private format
** NOTE: does not support encrypted private keys...
******************************************************************************/
bool oRSA::SetContextPrivatePEM(const char * pKey)
{
    // XXX - jbates - do not lose our padding and hash_id initialization...
    int padding = ctx.padding;
    int hash_id = ctx.hash_id;
    Auto_pk_context auto_pkctx;
    pk_context * pkctx = auto_pkctx.Ptr();
    if (!pkctx)
        return false;
    pk_init(pkctx);
    int rc = pk_parse_key(pkctx, reinterpret_cast<const unsigned char*>(pKey), strlen(pKey), NULL/*pwd*/, 0/*pwdlen*/);
    if (rc == 0)
    {
        rc = !(pkctx->pk_info->type == POLARSSL_PK_RSA);
        if (rc == 0)
        {
            // free any existing context
            rsa_free(&ctx);
            rsa_copy(&ctx, reinterpret_cast<rsa_context *>(pkctx->pk_ctx));
            ctx.padding = padding;
            ctx.hash_id = hash_id;
            // now we verify we're golden...
            rc = rsa_check_privkey(&ctx);
        }
    }
    return (rc == 0);
}

/******************************************************************************
** set the RSA public key via PEM public format
******************************************************************************/
bool oRSA::SetContextPublicPEM(const char * pKey)
{
    // XXX - jbates - do not lose our padding and hash_id initialization...
    int padding = ctx.padding;
    int hash_id = ctx.hash_id;
    Auto_pk_context auto_pkctx;
    pk_context * pkctx = auto_pkctx.Ptr();
    if (!pkctx)
        return false;
    pk_init(pkctx);
    int rc = pk_parse_public_key(pkctx, reinterpret_cast<const unsigned char*>(pKey), strlen(pKey));
    if (rc == 0)
    {
        rc = !(pkctx->pk_info->type == POLARSSL_PK_RSA);
        if (rc == 0)
        {
            // free any existing context
            rsa_free(&ctx);
            rsa_copy(&ctx, reinterpret_cast<rsa_context *>(pkctx->pk_ctx));
            ctx.padding = padding;
            ctx.hash_id = hash_id;
            // now we verify we're golden...
            rc = rsa_check_pubkey(&ctx);
        }
    }
    return (rc == 0);
}


/******************************************************************************
** set the RSA public key via PEM public format
******************************************************************************/
bool oRSA::GetRootCAPEM(const char * subject, uint32_t validity_days, char * PEM, u32 * rSize)
{
#if defined(_VDSP)
    return false;
#else
    if (!subject)
        return false;
    if (!strlen(subject))
        return false;
    if (validity_days > 365 * 20 + 5/*some leap years..??*/)
        return false;
    if (!PEM)
        return false;
    if (!rSize)
        return false;

    int rc = -1;
    // prepare dates for cert
    //struct timeval tv_now;
    //gettimeofday(&tv_now, NULL);
    time_t t_now;
    time(&t_now);
    //struct timeval tv_exp;
    //tv_exp.tv_sec = tv_now.tv_sec + (((uint64_t) validity_days) * 86400);
    time_t t_exp;
    t_exp = t_now + (((uint64_t) validity_days) * 86400);
    struct tm stm;
    gmtime_r(&t_now, &stm);
    char ts_now[16];
    strftime(ts_now, sizeof(ts_now), "%Y%m%d%H%M%S", &stm);
    gmtime_r(&t_exp, &stm);
    char ts_exp[16];
    strftime(ts_exp, sizeof(ts_exp), "%Y%m%d%H%M%S", &stm);

    // setup cert.
    Auto_x509write_cert Auto_cert;
    Auto_mpi auto_mpi_serial;
    Auto_pk_context auto_pk_ctx;
    x509write_cert * cert = Auto_cert.Ptr();
    mpi * serial = auto_mpi_serial.Ptr();
    pk_context * pk_ctx = auto_pk_ctx.Ptr();
    if (cert && serial && pk_ctx)
    {
        // prepare PK context with our RSA ctx as contents...
        pk_init(pk_ctx);
        pk_ctx->pk_info = pk_info_from_type(POLARSSL_PK_RSA);
        pk_ctx->pk_ctx = &ctx;

        // serial number for cert
        mpi_read_string(serial, 10, "1");

        x509write_crt_set_version(cert, X509_CRT_VERSION_3);
        x509write_crt_set_md_alg(cert, POLARSSL_MD_SHA256);
        x509write_crt_set_subject_key(cert, pk_ctx);
        x509write_crt_set_issuer_key(cert, pk_ctx);

                     rc = x509write_crt_set_subject_name(cert, subject);
        if (rc == 0) rc = x509write_crt_set_issuer_name(cert, subject);
        if (rc == 0) rc = x509write_crt_set_subject_key_identifier(cert);
        if (rc == 0) rc = x509write_crt_set_validity(cert, ts_now, ts_exp);
        if (rc == 0) rc = x509write_crt_set_serial(cert, serial);
        // NOTE: jbates - basic constraints (i.e. pathlen) are ignored in trust anchors like root CAs...
        if (rc == 0) rc = x509write_crt_set_basic_constraints(cert, 1/*is_ca*/, 2/*max_pathlen*/);
        if (rc == 0) rc = x509write_crt_set_key_usage(cert, KU_KEY_CERT_SIGN);
        if (rc == 0) rc = x509write_crt_pem(cert, reinterpret_cast<unsigned char *>(PEM), *rSize, lumi_random, NULL);
    }

    *rSize = 0;
    if (rc == 0)
        *rSize = static_cast<u32>(strlen(PEM) + 1);

    pk_ctx->pk_ctx = NULL;   // avoid horrible repercussions!
    pk_ctx->pk_info = NULL;

    return (rc == 0);
#endif
}

// ================================================================================================================================
//
bool oRSA::GetCertSignReqPEM(const char * subject, int key_usage, char * PEM, u32 * rSize)
{
    if (!subject)
        return false;
    if (!strlen(subject))
        return false;
    if (key_usage != KU_DIGITAL_SIGNATURE && key_usage != KU_KEY_ENCIPHERMENT && key_usage != KU_KEY_CERT_SIGN)
        return false;
    if (!PEM)
        return false;
    if (!rSize)
        return false;

    int rc = -1;

    Auto_x509write_csr auto_x509write_csr;
    Auto_pk_context auto_pk_ctx;
    x509write_csr * req = auto_x509write_csr.Ptr();
    pk_context * pk_ctx = auto_pk_ctx.Ptr();
    if (req && pk_ctx)
    {
        // prepare PK context with our RSA ctx as contents...
        pk_init(pk_ctx);
        pk_ctx->pk_info = pk_info_from_type(POLARSSL_PK_RSA);
        pk_ctx->pk_ctx = &ctx;

        x509write_csr_set_key(req, pk_ctx);
        x509write_csr_set_md_alg(req, POLARSSL_MD_SHA256);

                     rc = x509write_csr_set_key_usage(req, key_usage);
        if (rc == 0) rc = x509write_csr_set_subject_name(req, subject);
        if (rc == 0) rc = x509write_csr_pem(req, reinterpret_cast<unsigned char *>(PEM), *rSize, lumi_random, NULL);
    }

    *rSize = 0;
    if (rc == 0)
        *rSize = static_cast<u32>(strlen(PEM) + 1);

    pk_ctx->pk_ctx = NULL;   // avoid horrible repercussions!
    pk_ctx->pk_info = NULL;

    return (rc == 0);
}

// ================================================================================================================================
//
bool oRSA::Generate_Keypair_Sealed(void * p_rng, u32 nbits, int exponent,
                                   const unsigned char * EKEY,        size_t   EKEY_len,
                                   const unsigned char * MKEY,        size_t   MKEY_len,
                                         unsigned char * ENV_PEM_prv, size_t * ENV_PEM_prv_len,   // NULLs to disable prv
                                         unsigned char * ENV_PEM_pub, size_t * ENV_PEM_pub_len,   // NULLs to disable pub
                                         unsigned char * ENV_PEM_csr, size_t * ENV_PEM_csr_len,   // NULLs to disable csr
                                            const char * subject, int key_usage)                  // NULL/-1 to disable csr
{
    IENVELOPE * pEnvelope = ICypher::GetInstance()->GetEnvelope();
    if (!pEnvelope)
        return false;
    AutoHeapBuffer auto_PEM(2048);
    if (!auto_PEM.u8Ptr())
        return false;
    if (nbits != 2048)
        return false;
    if (exponent != 0x10001)
        return false;
    if (!subject)
        return false;
    if (key_usage != KU_DIGITAL_SIGNATURE && key_usage != KU_KEY_ENCIPHERMENT && key_usage != KU_KEY_CERT_SIGN)
        return false;

    // know buffer sizes, in case we need to clear them...
    size_t ENV_PEM_prv_in_len = 0;
    size_t ENV_PEM_pub_in_len = 0;
    size_t ENV_PEM_csr_in_len = 0;
    if (ENV_PEM_prv_len)
        ENV_PEM_prv_in_len = *ENV_PEM_prv_len;
    if (ENV_PEM_pub_len)
        ENV_PEM_pub_in_len = *ENV_PEM_pub_len;
    if (ENV_PEM_csr_len)
        ENV_PEM_csr_in_len = *ENV_PEM_csr_len;

    bool rc = false;

    rc = Generate_Keypair(p_rng, nbits, exponent);
    // generate a sealed envelope with the private key?
    if (rc && ENV_PEM_prv && ENV_PEM_prv_len && *ENV_PEM_prv_len)
    {
        u32 len = static_cast<u32>(auto_PEM.Len());
        rc = GetPrivateKeyPEM(auto_PEM.charPtr(), &len);
        if (rc)
            rc = pEnvelope->Seal(EKEY, EKEY_len, MKEY, MKEY_len, IENVELOPE::RSA_2048_PRIV_KEY, NULL/*counter - randomize*/,
                                 auto_PEM.u8Ptr(), len, ENV_PEM_prv, ENV_PEM_prv_len);
    }
    // generate a sealed envelope with the public key?
    if (rc && ENV_PEM_pub && ENV_PEM_pub_len && *ENV_PEM_pub_len)
    {
        u32 len = static_cast<u32>(auto_PEM.Len());
        rc = GetPublicKeyPEM(auto_PEM.charPtr(), &len);
        if (rc)
            rc = pEnvelope->Seal(EKEY, EKEY_len, MKEY, MKEY_len, IENVELOPE::RSA_2048_PUB_KEY, NULL/*counter - randomize*/,
                                 auto_PEM.u8Ptr(), len, ENV_PEM_pub, ENV_PEM_pub_len);
    }
    // generate a sealed envelope with the cert sign req?
    if (rc && ENV_PEM_csr && ENV_PEM_csr_len && *ENV_PEM_csr_len)
    {
        u32 len = static_cast<u32>( auto_PEM.Len());
        rc = GetCertSignReqPEM(subject, key_usage, auto_PEM.charPtr(), &len);
        if (rc)
            rc = pEnvelope->Seal(EKEY, EKEY_len, MKEY, MKEY_len, IENVELOPE::RSA_2048_CERT_REQ, NULL/*counter - randomize*/,
                                 auto_PEM.u8Ptr(), len, ENV_PEM_csr, ENV_PEM_csr_len);
    }

    // if we were not successful, be forceful!
    if (!rc)
    {
        if (ENV_PEM_prv_in_len)
        {
            *ENV_PEM_prv_len = 0;
            memset(ENV_PEM_prv, 0, ENV_PEM_prv_in_len);
        }
        if (ENV_PEM_pub_in_len)
        {
            *ENV_PEM_pub_len = 0;
            memset(ENV_PEM_pub, 0, ENV_PEM_pub_in_len);
        }
        if (ENV_PEM_csr_in_len)
        {
            *ENV_PEM_csr_len = 0;
            memset(ENV_PEM_csr, 0, ENV_PEM_csr_in_len);
        }
    }

    return rc;
}

// ================================================================================================================================
//
bool oRSA::SetContextPrivateSealed(const unsigned char * EKEY,           size_t   EKEY_len,
                                   const unsigned char * MKEY,           size_t   MKEY_len,
                                   const unsigned char * ENV_sealed_prv, size_t   ENV_sealed_prv_len)
{
    IENVELOPE * pEnvelope = ICypher::GetInstance()->GetEnvelope();
    if (!pEnvelope)
        return false;
    AutoHeapBuffer auto_PEM(2048);
    if (!auto_PEM.u8Ptr())
        return false;

    bool rc = false;

    size_t len = auto_PEM.Len();
    rc = pEnvelope->Unseal(EKEY, EKEY_len, MKEY, MKEY_len, IENVELOPE::RSA_2048_PRIV_KEY, NULL/*counter - accept any*/,
                           ENV_sealed_prv, ENV_sealed_prv_len, auto_PEM.u8Ptr(), &len);
    if (rc)
    {
        //static const char * BEGIN_DELIMITER = "-----BEGIN RSA PRIVATE KEY-----";
        //static const char *   END_DELIMITER = "-----END RSA PRIVATE KEY-----";
        rc = 0; //Validate_PEM_Entity(auto_PEM.charPtr(), len, BEGIN_DELIMITER, END_DELIMITER);
        if (rc == 0)
        {
            rc = SetContextPrivatePEM(auto_PEM.charPtr());
        }
    }

    return rc;
}

// ================================================================================================================================
//
bool oRSA::SetContextPublicSealed (const unsigned char * EKEY,           size_t   EKEY_len,
                                   const unsigned char * MKEY,           size_t   MKEY_len,
                                   const unsigned char * ENV_sealed_pub, size_t   ENV_sealed_pub_len)
{
    IENVELOPE * pEnvelope = ICypher::GetInstance()->GetEnvelope();
    if (!pEnvelope)
        return false;
    AutoHeapBuffer auto_PEM(2048);
    if (!auto_PEM.u8Ptr())
        return false;

    bool rc = false;

    size_t len = auto_PEM.Len();
    rc = pEnvelope->Unseal(EKEY, EKEY_len, MKEY, MKEY_len, IENVELOPE::RSA_2048_PUB_KEY, NULL/*counter - accept any*/,
                           ENV_sealed_pub, ENV_sealed_pub_len, auto_PEM.u8Ptr(), &len);
    if (rc)
    {
        //static const char * BEGIN_DELIMITER = "-----BEGIN PUBLIC KEY-----";
        //static const char *   END_DELIMITER = "-----END PUBLIC KEY-----";
        rc = 0; //Validate_PEM_Entity(auto_PEM.charPtr(), len, BEGIN_DELIMITER, END_DELIMITER);
        if (rc == 0)
        {
            rc = SetContextPublicPEM(auto_PEM.charPtr());
        }
    }

    return rc;
}

// ================================================================================================================================
