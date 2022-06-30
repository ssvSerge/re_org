#pragma once

/*******************************************************************************
**
**	X509 - x.509 functionality within ICypher
**	$Id: IBSP.h 22877 2014-03-03 03:38:32Z alitz $
**
**	COPYRIGHT INFORMATION:	
**		This software is proprietary and confidential.  
**		By using this software you agree to the terms and conditions of the 
**		associated Lumidigm Inc. License Agreement.
**
**		(c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include "ICypher.h"
#include "IMemMgr.h"

class oX509 : public IX509, public MemoryBase
{
public:
                 oX509() {}
    virtual      ~oX509() {}

    // Unseal various X.509 entities
    virtual bool UnsealRootCA(const unsigned char * EKEY,       size_t   EKEY_len,
                              const unsigned char * MKEY,       size_t   MKEY_len,
                              const unsigned char * CertSealed, size_t   CertSealedLen,
                                             char * CertPEM,    size_t * CertPEMLen);

    virtual bool UnsealInterCA(const unsigned char * EKEY,       size_t   EKEY_len,
                               const unsigned char * MKEY,       size_t   MKEY_len,
                               const unsigned char * CertSealed, size_t   CertSealedLen,
                                              char * CertPEM,    size_t * CertPEMLen);

    virtual bool UnsealLeafCert(const unsigned char * EKEY,       size_t   EKEY_len,
                                const unsigned char * MKEY,       size_t   MKEY_len,
                                const unsigned char * CertSealed, size_t   CertSealedLen,
                                               char * CertPEM,    size_t * CertPEMLen);

    virtual bool UnsealCertSignReq(const unsigned char * EKEY,              size_t   EKEY_len,
                                   const unsigned char * MKEY,              size_t   MKEY_len,
                                   const unsigned char * CertSignReqSealed, size_t   CertSignReqSealedLen,
                                                  char * CertSignReqPEM,    size_t * CertSignReqPEMLen);

    // Seal various X.509 entities
    virtual bool SealRootCA(const unsigned char * EKEY,       size_t   EKEY_len,
                            const unsigned char * MKEY,       size_t   MKEY_len,
                            const          char * CertPEM,    size_t   CertPEMLen,
                                  unsigned char * CertSealed, size_t * CertSealedLen);

    virtual bool SealInterCA(const unsigned char * EKEY,       size_t   EKEY_len,
                             const unsigned char * MKEY,       size_t   MKEY_len,
                             const          char * CertPEM,    size_t   CertPEMLen,
                                   unsigned char * CertSealed, size_t * CertSealedLen);


    virtual bool SealLeafCert(const unsigned char * EKEY,       size_t   EKEY_len,
                              const unsigned char * MKEY,       size_t   MKEY_len,
                              const          char * CertPEM,    size_t   CertPEMLen,
                                    unsigned char * CertSealed, size_t * CertSealedLen);

    virtual bool SealCertSignReq(const unsigned char * EKEY,              size_t   EKEY_len,
                                 const unsigned char * MKEY,              size_t   MKEY_len,
                                 const          char * CertSignReqPEM,    size_t   CertSignReqPEMLen,
                                       unsigned char * CertSignReqSealed, size_t * CertSignReqSealedLen);

    // NOTE: PolarSSL does not support this, and we have not written our own yet!
    virtual bool VerifyCertSignReq(const char * CertReqPEM, size_t CertReqPEMLen);

	// SIGN a certificate request with a CA cert - using plaintext private key and producing plaintext output
	virtual bool SignCertReq(const          char * CertReqPEM,		 size_t   CertReqPEMLen,
							 const          char * SigningCertPEM,	 size_t   SigningCertPEMLen,
							 const          char * PrivateKeyPEM,	 size_t   PrivateKeyPEMLen,
							 uint32_t              validity_days,	 uint32_t serial,
							 int                   key_usage,         bool CA, int max_pathlen,
							               char * CertSignedPEM,	 size_t * CertSignedPEMLen);

    // SIGN a certificate request with a CA cert - using sealed private key and producing sealed output
    virtual bool SignCertReq(const unsigned char * EKEY,             size_t   EKEY_len,
                             const unsigned char * MKEY,             size_t   MKEY_len,
                             const          char * CertReqPEM,       size_t   CertReqPEMLen,
                             const          char * SigningCertPEM,   size_t   SigningCertPEMLen,
                             const unsigned char * PrivateKeySealed, size_t   PrivateKeySealedLen,
                             uint32_t              validity_days,    uint32_t serial,
                             int                   key_usage,        uint32_t mid,
                                   unsigned char * SignedCertSealed, size_t * SignedCertSealedLen);

    // VERIFY a certificate chain
    // RootCAPEM should be a self-signed trust anchor
    // CertChainPEM should be all remaining certs in the chain, in the order of creation/signing
    // i.e. just a leaf cert -or- an intermediate CA cert, then a leaf cert
    virtual bool VerifyCertChain(const char * RootCAPEM,    size_t RootCAPEMLen,
                                 const char * CertChainPEM, size_t CertChainPEMLen);

    virtual bool CertReqGetPublicKeyPEM(const char * CertReqPEM,   size_t   CertReqPEMLen,
                                              char * PublicKeyPEM, size_t * PublicKeyPEMLen);

    virtual bool CertGetPublicKeyPEM   (const char * CertPEM,      size_t   CertPEMLen,
                                              char * PublicKeyPEM, size_t * PublicKeyPEMLen);

private:
    virtual bool Unseal(const unsigned char * EKEY,   size_t   EKEY_len,
                        const unsigned char * MKEY,   size_t   MKEY_len,
                        const unsigned char * Sealed, size_t   SealedLen,
                        uint32_t              mid,
                                       char * PEM,    size_t * PEMLen);
    virtual bool Seal(const unsigned char * EKEY,   size_t   EKEY_len,
                      const unsigned char * MKEY,   size_t   MKEY_len,
                      const          char * PEM,    size_t   PEMLen,
                      uint32_t              mid,
                            unsigned char * Sealed, size_t * SealedLen);
};
