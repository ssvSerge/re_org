/*******************************************************************************
**
**    X509 - x.509 functionality within ICypher
**    $Id: IBSP.h 22877 2014-03-03 03:38:32Z alitz $
**
**    COPYRIGHT INFORMATION:    
**        This software is proprietary and confidential.  
**        By using this software you agree to the terms and conditions of the 
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include "Cypher.h"
#include "Auto_Crypto_Objects.h"
#include "lumi_random.h"

bool oX509::VerifyCertSignReq(const char * CertReqPEM, size_t CertReqPEMLen)
{
    return false;
}

bool oX509::Unseal(const unsigned char * EKEY,   size_t   EKEY_len,
                   const unsigned char * MKEY,   size_t   MKEY_len,
                   const unsigned char * Sealed, size_t   SealedLen,
                   uint32_t              mid,
                                  char * PEM,    size_t * PEMLen)
{
    if (!Sealed)
        return false;
    if (!PEM)
        return false;
    if (!PEMLen)
        return false;

    IENVELOPE * pEnvelope = ICypher::GetInstance()->GetEnvelope();

    bool rc = false;

    rc = pEnvelope->Unseal(EKEY, EKEY_len, MKEY, MKEY_len, mid, NULL/*counter - accept any*/,
                           Sealed, SealedLen, reinterpret_cast<unsigned char *>(PEM), PEMLen);
    if (rc)
    {
    }

    return rc;
}

bool oX509::UnsealRootCA(const unsigned char * EKEY,       size_t   EKEY_len,
                         const unsigned char * MKEY,       size_t   MKEY_len,
                         const unsigned char * CertSealed, size_t   CertSealedLen,
                                        char * CertPEM,    size_t * CertPEMLen)
{
    return Unseal(EKEY, EKEY_len, MKEY, MKEY_len, CertSealed, CertSealedLen, IENVELOPE::RSA_2048_ROOT_CA, CertPEM, CertPEMLen);
}

bool oX509::UnsealInterCA(const unsigned char * EKEY,       size_t   EKEY_len,
                          const unsigned char * MKEY,       size_t   MKEY_len,
                          const unsigned char * CertSealed, size_t   CertSealedLen,
                                         char * CertPEM,    size_t * CertPEMLen)
{
    return Unseal(EKEY, EKEY_len, MKEY, MKEY_len, CertSealed, CertSealedLen, IENVELOPE::RSA_2048_INTER_CA, CertPEM, CertPEMLen);
}

bool oX509::UnsealLeafCert(const unsigned char * EKEY,       size_t   EKEY_len,
                           const unsigned char * MKEY,       size_t   MKEY_len,
                           const unsigned char * CertSealed, size_t   CertSealedLen,
                                          char * CertPEM,    size_t * CertPEMLen)
{
    return Unseal(EKEY, EKEY_len, MKEY, MKEY_len, CertSealed, CertSealedLen, IENVELOPE::RSA_2048_LEAF_CERT, CertPEM, CertPEMLen);
}

bool oX509::UnsealCertSignReq(const unsigned char * EKEY,              size_t   EKEY_len,
                              const unsigned char * MKEY,              size_t   MKEY_len,
                              const unsigned char * CertSignReqSealed, size_t   CertSignReqSealedLen,
                                             char * CertSignReqPEM,    size_t * CertSignReqPEMLen)
{
    return Unseal(EKEY, EKEY_len, MKEY, MKEY_len, CertSignReqSealed, CertSignReqSealedLen, IENVELOPE::RSA_2048_CERT_REQ,
                                                  CertSignReqPEM,    CertSignReqPEMLen);
}

bool oX509::Seal(const unsigned char * EKEY,   size_t   EKEY_len,
                 const unsigned char * MKEY,   size_t   MKEY_len,
                 const          char * PEM,    size_t   PEMLen,
                 uint32_t              mid,
                       unsigned char * Sealed, size_t * SealedLen)
{
    if (!Sealed)
        return false;
    if (!PEM)
        return false;
    if (!SealedLen)
        return false;

    IENVELOPE * pEnvelope = ICypher::GetInstance()->GetEnvelope();

    bool rc = false;

    rc = pEnvelope->Seal(EKEY, EKEY_len, MKEY, MKEY_len, mid, NULL/*counter - accept any*/,
                         reinterpret_cast<const unsigned char *>(PEM), PEMLen, Sealed, SealedLen);
    if (rc)
    {
    }

    return rc;
}

bool oX509::SealRootCA(const unsigned char * EKEY,       size_t   EKEY_len,
                            const unsigned char * MKEY,       size_t   MKEY_len,
                            const          char * CertPEM,    size_t   CertPEMLen,
                                  unsigned char * CertSealed, size_t * CertSealedLen)
{
    return Seal(EKEY, EKEY_len, MKEY, MKEY_len, CertPEM, CertPEMLen, IENVELOPE::RSA_2048_ROOT_CA, CertSealed, CertSealedLen);
}

bool oX509::SealInterCA(const unsigned char * EKEY,       size_t   EKEY_len,
                             const unsigned char * MKEY,       size_t   MKEY_len,
                             const          char * CertPEM,    size_t   CertPEMLen,
                                   unsigned char * CertSealed, size_t * CertSealedLen)
{
    return Seal(EKEY, EKEY_len, MKEY, MKEY_len, CertPEM, CertPEMLen, IENVELOPE::RSA_2048_INTER_CA, CertSealed, CertSealedLen);
}

bool oX509::SealLeafCert(const unsigned char * EKEY,       size_t   EKEY_len,
                              const unsigned char * MKEY,       size_t   MKEY_len,
                              const          char * CertPEM,    size_t   CertPEMLen,
                                    unsigned char * CertSealed, size_t * CertSealedLen)
{
    return Seal(EKEY, EKEY_len, MKEY, MKEY_len, CertPEM, CertPEMLen, IENVELOPE::RSA_2048_LEAF_CERT, CertSealed, CertSealedLen);
}

bool oX509::SealCertSignReq(const unsigned char * EKEY,              size_t   EKEY_len,
                                 const unsigned char * MKEY,              size_t   MKEY_len,
                                 const          char * CertSignReqPEM,    size_t   CertSignReqPEMLen,
                                       unsigned char * CertSignReqSealed, size_t * CertSignReqSealedLen)
{
    return Seal(EKEY, EKEY_len, MKEY, MKEY_len, CertSignReqPEM,    CertSignReqPEMLen, IENVELOPE::RSA_2048_CERT_REQ,
                                                CertSignReqSealed, CertSignReqSealedLen);
}

bool oX509::SignCertReq(const          char * CertReqPEM,        size_t   CertReqPEMLen,
                        const          char * SigningCertPEM,    size_t   SigningCertPEMLen,
                        const          char * PrivateKeyPEM,    size_t   PrivateKeyPEMLen,
                        uint32_t              validity_days,    uint32_t serial,
                        int                   key_usage,        bool CA, int max_pathlen,
                                       char * CertSignedPEM,    size_t * CertSignedPEMLen)
{
    //IRSA * pRSA = ICypher::GetInstance()->GetRSA();
    //IX509 * pX509 = ICypher::GetInstance()->GetX509();

    //bool CA;
    //int max_pathlen;

    Auto_x509_csr auto_x509_csr;
    x509_csr * csr = auto_x509_csr.Ptr();
    if (!csr)
        return false;
    Auto_x509_crt auto_x509_crt_issuer;
    x509_crt * crt_issuer = auto_x509_crt_issuer.Ptr();
    if (!crt_issuer)
        return false;
    Auto_pk_context auto_pk_issuer;
    pk_context * pk_issuer = auto_pk_issuer.Ptr();
    if (!pk_issuer)
        return false;
    Auto_x509write_cert auto_crt_subject;
    x509write_cert * crt_subject = auto_crt_subject.Ptr();
    if (!crt_subject)
        return false;
    Auto_mpi auto_mpi_serial;
    mpi * mpi_serial = auto_mpi_serial.Ptr();
    if (!mpi_serial)
        return false;
    if (mpi_lset(mpi_serial, serial) != 0)
        return false;

    if (x509_csr_parse(csr, reinterpret_cast<const unsigned char *>(CertReqPEM), CertReqPEMLen) != 0)
        return false;
    if (x509_crt_parse(crt_issuer, reinterpret_cast<const unsigned char *>(SigningCertPEM), SigningCertPEMLen) != 0)
        return false;
    uint32_t PrivateKeyLen = static_cast<u32>(PrivateKeyPEMLen);
    if (pk_parse_key(pk_issuer, reinterpret_cast<const unsigned char *>(PrivateKeyPEM), PrivateKeyLen, NULL/*pwd*/, 0/*pwdlen*/) != 0)
        return false;

    x509write_crt_set_version(crt_subject, X509_CRT_VERSION_3);
    x509write_crt_set_md_alg(crt_subject, POLARSSL_MD_SHA256);
    x509write_crt_set_subject_key(crt_subject, &(csr->pk));
    x509write_crt_set_issuer_key(crt_subject, pk_issuer);

    AutoHeapBuffer subject_name(csr->subject.val.len + 3/*CN=*/ + 1/*NUL*/);
    if (!subject_name.charPtr())
        return false;
    strcpy(subject_name.charPtr(), "CN=");
    memcpy(subject_name.charPtr() + 3, csr->subject.val.p, csr->subject.val.len);
    subject_name.charPtr()[subject_name.Len() - 1] = '\0';
    AutoHeapBuffer issuer_name(crt_issuer->subject.val.len + 3/*CN=*/ + 1/*NUL*/);
    if (!issuer_name.charPtr())
        return false;
    strcpy(issuer_name.charPtr(), "CN=");
    memcpy(issuer_name.charPtr() + 3, crt_issuer->subject.val.p, crt_issuer->subject.val.len);
    issuer_name.charPtr()[issuer_name.Len() - 1] = '\0';
    //fprintf(stdout, "ISSUER_NAME[%d]: %s", csr->subject.val.len, issuer_name.charPtr());

    if (x509write_crt_set_subject_name(crt_subject, subject_name.charPtr()))//reinterpret_cast<char *>(csr->subject_raw.p)) != 0)
        return false;
    if (x509write_crt_set_issuer_name(crt_subject, issuer_name.charPtr())) //reinterpret_cast<char *>(crt_issuer->subject_raw.p)) != 0)
        return false;
    if (x509write_crt_set_subject_key_identifier(crt_subject) != 0)
        return false;
    if (x509write_crt_set_validity(crt_subject, "20150101000000", "20250101000000") != 0)
        return false;
    if (x509write_crt_set_serial(crt_subject, mpi_serial) != 0)
        return false;
    // NOTE: jbates - basic constraints (i.e. pathlen) are ignored in trust anchors like root CAs...
    if (x509write_crt_set_basic_constraints(crt_subject, CA/*is_ca*/, max_pathlen) != 0)
        return false;
    if (x509write_crt_set_key_usage(crt_subject, key_usage) != 0)
        return false;

    bool rc = x509write_crt_pem(crt_subject, reinterpret_cast<u8 *>(CertSignedPEM), *CertSignedPEMLen, lumi_random, NULL) == 0;
    if (rc)
        *CertSignedPEMLen = strlen(CertSignedPEM);
    else
        *CertSignedPEMLen = 0;

    return rc;
}

bool oX509::SignCertReq(const unsigned char * EKEY,             size_t   EKEY_len,
                        const unsigned char * MKEY,             size_t   MKEY_len,
                        const          char * CertReqPEM,       size_t   CertReqPEMLen,
                        const          char * SigningCertPEM,   size_t   SigningCertPEMLen,
                        const unsigned char * PrivateKeySealed, size_t   PrivateKeySealedLen,
                        uint32_t              validity_days,    uint32_t serial,
                        int                   key_usage,        uint32_t mid,
                              unsigned char * SignedCertSealed, size_t * SignedCertSealedLen)
{
    bool CA;
    int max_pathlen;

    switch (mid)
    {
    case IENVELOPE::RSA_2048_ROOT_CA:
        CA = true; max_pathlen = 2; break;
    case IENVELOPE::RSA_2048_INTER_CA:
        CA = true; max_pathlen = 1; break;
    case IENVELOPE::RSA_2048_LEAF_CERT:
        CA = false; max_pathlen = 0; break;
    default:
        return false;
    }
    IRSA * pRSA = ICypher::GetInstance()->GetRSA();
    AutoHeapBuffer PrivateKeyPEM(IENVELOPE::RSA_2048_PEM_PRV_SIZE);
    if (!PrivateKeyPEM.u8Ptr())
        return false;
    AutoHeapBuffer CertSignedPEM(IENVELOPE::RSA_2048_PEM_CRT_SIZE);
    if (!CertSignedPEM.u8Ptr())
        return false;
    if (!pRSA->SetContextPrivateSealed(EKEY, EKEY_len, MKEY, MKEY_len, PrivateKeySealed, PrivateKeySealedLen))
        return false;
    uint32_t PrivateKeyLen = static_cast<u32>(PrivateKeyPEM.Len());
    if (!pRSA->GetPrivateKeyPEM(PrivateKeyPEM.charPtr(), reinterpret_cast<u32 *>(&PrivateKeyLen)))
        return false;
    size_t CertSignedPEMLen = CertSignedPEM.Len();
    bool rc = SignCertReq(CertReqPEM, CertReqPEMLen,
                          SigningCertPEM, SigningCertPEMLen, 
                          PrivateKeyPEM.charPtr(), strlen(PrivateKeyPEM.charPtr()),
                          validity_days, serial, key_usage, CA, max_pathlen,
                          CertSignedPEM.charPtr(), &CertSignedPEMLen);
    if (rc)
    {
        size_t pem_len = strlen(CertSignedPEM.charPtr()) + 1;
        rc = Seal(EKEY, EKEY_len, MKEY, MKEY_len,
                  CertSignedPEM.charPtr(), pem_len,
                  mid,
                  SignedCertSealed, SignedCertSealedLen);
        if (rc)
        {
        }
    }

    return rc;
}

bool oX509::VerifyCertChain(const char * RootCAPEM, size_t RootCAPEMLen, const char * CertChainPEM, size_t CertChainPEMLen)
{
    // validate cert chain to root...
    int rc = -1;

    Auto_x509_crt auto_x509_crt_root_ca;
    Auto_x509_crt auto_x509_crt_chain;

    rc = auto_x509_crt_root_ca.Ptr()  == NULL ||
         auto_x509_crt_chain.Ptr()    == NULL;

    if (rc == 0)
    {
        rc = x509_crt_parse(auto_x509_crt_root_ca.Ptr(),
                            reinterpret_cast<const unsigned char *>(RootCAPEM), RootCAPEMLen);
        if (rc == 0)
        {
            rc = x509_crt_parse(auto_x509_crt_chain.Ptr(),
                                reinterpret_cast<const unsigned char *>(CertChainPEM), CertChainPEMLen);
            if (rc == 0)
            {
                int flags = 0;
                rc = x509_crt_verify(auto_x509_crt_chain.Ptr(),     // chain to verify
                                     auto_x509_crt_root_ca.Ptr(),   // trusted CA chain
                                     NULL,                          // ca_crl
                                     NULL,                          // cn
                                     &flags,
                                     NULL,                          // f_vrfy (callback)
                                     NULL);                         // p_vrfy (callback context)
            }
            else {}
        }
        else {}
    }
    else {}

    return (rc == 0);
}

bool oX509::CertGetPublicKeyPEM(const char * CertPEM,      size_t   CertPEMLen,
                                      char * PublicKeyPEM, size_t * PublicKeyPEMLen)
{
    // validate cert chain to root...
    int rc = -1;

    Auto_x509_crt auto_x509_crt;

    rc = auto_x509_crt.Ptr() == NULL;

    if (rc == 0)
    {
        rc = x509_crt_parse(auto_x509_crt.Ptr(), reinterpret_cast<const unsigned char *>(CertPEM), CertPEMLen);
        if (rc == 0)
        {
            rc = pk_write_pubkey_pem(&(auto_x509_crt.Ptr()->pk), reinterpret_cast<unsigned char *>(PublicKeyPEM),
                                     *PublicKeyPEMLen);
            if (rc == 0)
                *PublicKeyPEMLen = strlen(PublicKeyPEM);
            else
                *PublicKeyPEMLen = 0;
        }
    }

    return !rc;
}

bool oX509::CertReqGetPublicKeyPEM(const char * CertReqPEM,   size_t   CertReqPEMLen,
                                         char * PublicKeyPEM, size_t * PublicKeyPEMLen)
{
    // validate cert chain to root...
    int rc = -1;

    Auto_x509_csr auto_x509_csr;

    rc = auto_x509_csr.Ptr() == NULL;

    if (rc == 0)
    {
        rc = x509_csr_parse(auto_x509_csr.Ptr(), reinterpret_cast<const unsigned char *>(CertReqPEM), CertReqPEMLen);
        if (rc == 0)
        {
            rc = pk_write_pubkey_pem(&(auto_x509_csr.Ptr()->pk), reinterpret_cast<unsigned char *>(PublicKeyPEM),
                                     *PublicKeyPEMLen);
            if (rc == 0)
                *PublicKeyPEMLen = strlen(PublicKeyPEM);
            else
                *PublicKeyPEMLen = 0;
        }
    }

    return !rc;
}

