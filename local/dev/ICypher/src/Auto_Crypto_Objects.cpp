// ================================================================================================================================
//
// Auto_Crypto_Objects.h
//
// Automate destruction of polarssl crypto object context structures based upon leaving scope, RAII-style.
//
// ================================================================================================================================

#include "Auto_Crypto_Objects.h"
#include "PlatformDev.h"

// ================================================================================================================================
// Auto_des_context
// these items help implement a smart pointer for des_context objects for PolarSSL/mbedtls.
des_context * Create_des_context()
{
    des_context * ctx = reinterpret_cast<des_context *>(MALLOC(sizeof(des_context)));
    if (ctx)
        des_init(ctx);
    return ctx;
}
void Delete_des_context(des_context * ctx)
{
    des_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_des3_context
// these items help implement a smart pointer for des3_context objects for PolarSSL/mbedtls.
des3_context * Create_des3_context()
{
    des3_context * ctx = reinterpret_cast<des3_context *>(MALLOC(sizeof(des3_context)));
    if (ctx)
        des3_init(ctx);
    return ctx;
}
void Delete_des3_context(des3_context * ctx)
{
    des3_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_aes_context
// these items help implement a smart pointer for aes_context objects for PolarSSL/mbedtls.
aes_context * Create_aes_context()
{
    aes_context * ctx = reinterpret_cast<aes_context *>(MALLOC(sizeof(aes_context)));
    if (ctx)
        aes_init(ctx);
    return ctx;
}
void Delete_aes_context(aes_context * ctx)
{
    aes_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_pk_context
// these items help implement a smart pointer for pk_context objects for PolarSSL/mbedtls.
pk_context * Create_pk_context()
{
    pk_context * ctx = reinterpret_cast<pk_context *>(MALLOC(sizeof(pk_context)));
    if (ctx)
        pk_init(ctx);
    return ctx;
}
void Delete_pk_context(pk_context * ctx)
{
    pk_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_mpi_context
// these items help implement a smart pointer for mpi_context objects for PolarSSL/mbedtls.
mpi * Create_mpi()
{
    mpi * ctx = reinterpret_cast<mpi *>(MALLOC(sizeof(mpi)));
    if (ctx)
        mpi_init(ctx);
    return ctx;
}
void Delete_mpi(mpi * ctx)
{
    mpi_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_rsa_context
// these items help implement a smart pointer for rsa_context objects for PolarSSL/mbedtls.
rsa_context * Create_rsa_context()
{
    rsa_context * ctx = reinterpret_cast<rsa_context *>(MALLOC(sizeof(rsa_context)));
    if (ctx)
        rsa_init(ctx, RSA_PKCS_V21, POLARSSL_MD_SHA256);
    return ctx;
}
void Delete_rsa_context(rsa_context * ctx)
{
    rsa_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_sha1_context
// these items help implement a smart pointer for sha1_context objects for PolarSSL/mbedtls.
sha1_context * Create_sha1_context()
{
    sha1_context * ctx = reinterpret_cast<sha1_context *>(MALLOC(sizeof(sha1_context)));
    if (ctx)
        sha1_init(ctx);
    return ctx;
}
void Delete_sha1_context(sha1_context * ctx)
{
    sha1_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_sha256_context
// these items help implement a smart pointer for sha256_context objects for PolarSSL/mbedtls.
sha256_context * Create_sha256_context()
{
    sha256_context * ctx = reinterpret_cast<sha256_context *>(MALLOC(sizeof(sha256_context)));
    if (ctx)
        sha256_init(ctx);
    return ctx;
}
void Delete_sha256_context(sha256_context * ctx)
{
    sha256_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_sha512_context
// these items help implement a smart pointer for sha512_context objects for PolarSSL/mbedtls.
sha512_context * Create_sha512_context()
{
    sha512_context * ctx = reinterpret_cast<sha512_context *>(MALLOC(sizeof(sha512_context)));
    if (ctx)
        sha512_init(ctx);
    return ctx;
}
void Delete_sha512_context(sha512_context * ctx)
{
    sha512_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_x509_csr
// these items help implement a smart pointer for x509_csr objects for PolarSSL/mbedtls.
x509_csr * Create_x509_csr()
{
    x509_csr * ctx = reinterpret_cast<x509_csr *>(MALLOC(sizeof(x509_csr)));
    if (ctx)
        x509_csr_init(ctx);
    return ctx;
}
void Delete_x509_csr(x509_csr * ctx)
{
    x509_csr_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_x509_crt
// these items help implement a smart pointer for x509_crt objects for PolarSSL/mbedtls.
x509_crt * Create_x509_crt()
{
    x509_crt * ctx = reinterpret_cast<x509_crt *>(MALLOC(sizeof(x509_crt)));
    if (ctx)
        x509_crt_init(ctx);
    return ctx;
}
void Delete_x509_crt(x509_crt * ctx)
{
    x509_crt_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_x509write_csr
// these items help implement a smart pointer for x509write_csr objects for PolarSSL/mbedtls.
x509write_csr * Create_x509write_csr()
{
    x509write_csr * ctx = reinterpret_cast<x509write_csr *>(MALLOC(sizeof(x509write_csr)));
    if (ctx)
        x509write_csr_init(ctx);
    return ctx;
}
void Delete_x509write_csr(x509write_csr * ctx)
{
    x509write_csr_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
// Auto_x509write_csr
// these items help implement a smart pointer for x509write_cert objects for PolarSSL/mbedtls.
x509write_cert * Create_x509write_cert()
{
    x509write_cert * ctx = reinterpret_cast<x509write_cert *>(MALLOC(sizeof(x509write_cert)));
    if (ctx)
        x509write_crt_init(ctx);
    return ctx;
}
void Delete_x509write_cert(x509write_cert * ctx)
{
    x509write_crt_free(ctx);
    SecureClearBuffer(ctx, sizeof(*ctx));
    FREE(ctx);
}

// ================================================================================================================================
