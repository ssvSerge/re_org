// ================================================================================================================================
//
// Auto_Crypto_Objects.h
//
// Automate destruction of polarssl crypto object context structures based upon leaving scope, RAII-style.
//
// ================================================================================================================================
#pragma once

#include "Platform.h"
#include "AutoHeapBuffer.h"

#include "polarssl/aes.h"
#include "polarssl/des.h"
#include "polarssl/bignum.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/md.h"
#include "polarssl/pk.h"
#include "polarssl/rsa.h"
#include "polarssl/sha1.h"
#include "polarssl/sha256.h"
#include "polarssl/sha512.h"
#include "polarssl/x509_csr.h"
#include "polarssl/x509_crt.h"

// ================================================================================================================================
// externally stored/initialized AES_CTR_DRBG
// see Platform.h / Platform.cpp
extern ctr_drbg_context g_aes_ctr_drbg_ctx;

// DES CONTEXT
extern des_context * Create_des_context();
extern void Delete_des_context(des_context * ctx);
typedef AutoHeapObject<des_context, Create_des_context, Delete_des_context> Auto_des_context;

// DES3 CONTEXT
extern des3_context * Create_des3_context();
extern void Delete_des3_context(des3_context * ctx);
typedef AutoHeapObject<des3_context, Create_des3_context, Delete_des3_context> Auto_des3_context;

// AES CONTEXT
extern aes_context * Create_aes_context();
extern void Delete_aes_context(aes_context * ctx);
typedef AutoHeapObject<aes_context, Create_aes_context, Delete_aes_context> Auto_aes_context;

// PK CONTEXT
extern pk_context * Create_pk_context();
extern void Delete_pk_context(pk_context * ctx);
typedef AutoHeapObject<pk_context, Create_pk_context, Delete_pk_context> Auto_pk_context;

// RSA CONTEXT
extern rsa_context * Create_rsa_context();
extern void Delete_rsa_context(rsa_context * ctx);
typedef AutoHeapObject<rsa_context, Create_rsa_context, Delete_rsa_context> Auto_rsa_context;

// SHA1 CONTEXT
extern sha1_context * Create_sha1_context();
extern void Delete_sha1_context(sha1_context * ctx);
typedef AutoHeapObject<sha1_context, Create_sha1_context, Delete_sha1_context> Auto_sha1_context;

// SHA256 CONTEXT
extern sha256_context * Create_sha256_context();
extern void Delete_sha256_context(sha256_context * ctx);
typedef AutoHeapObject<sha256_context, Create_sha256_context, Delete_sha256_context> Auto_sha256_context;

// SHA512 CONTEXT
extern sha512_context * Create_sha512_context();
extern void Delete_sha512_context(sha512_context * ctx);
typedef AutoHeapObject<sha512_context, Create_sha512_context, Delete_sha512_context> Auto_sha512_context;

// X509 CSR CONTEXT
extern x509_csr * Create_x509_csr();
extern void Delete_x509_csr(x509_csr * ctx);
typedef AutoHeapObject<x509_csr, Create_x509_csr, Delete_x509_csr> Auto_x509_csr;

// X509 CRT CONTEXT
extern x509_crt * Create_x509_crt();
extern void Delete_x509_crt(x509_crt * ctx);
typedef AutoHeapObject<x509_crt, Create_x509_crt, Delete_x509_crt> Auto_x509_crt;

// CSR WRITER CONTEXT
extern x509write_csr * Create_x509write_csr();
extern void Delete_x509write_csr(x509write_csr * ctx);
typedef AutoHeapObject<x509write_csr, Create_x509write_csr, Delete_x509write_csr> Auto_x509write_csr;

// CERT WRITER CONTEXT
extern x509write_cert * Create_x509write_cert();
extern void Delete_x509write_cert(x509write_cert * ctx);
typedef AutoHeapObject<x509write_cert, Create_x509write_cert, Delete_x509write_cert> Auto_x509write_cert;

// MPI WRITER CONTEXT
extern mpi * Create_mpi();
extern void Delete_mpi(mpi * ctx);
typedef AutoHeapObject<mpi, Create_mpi, Delete_mpi> Auto_mpi;

// ================================================================================================================================
