/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_rsa.h 22877 2014-03-03 03:38:32Z spcorcoran $
**
**	COPYRIGHT INFORMATION:	
**		This software is proprietary and confidential.  
**		By using this software you agree to the terms and conditions of the 
**		associated Lumidigm Inc. License Agreement.
**
**		(c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
#pragma once
#include "IMemMgr.h"
#include "CypherTypes.h"
#include "polarssl/rsa.h"

class oRSA : public IRSA,  public MemoryBase
{
public:

             oRSA();
    virtual ~oRSA();
	
	virtual bool Init(int padding,  int hash_id);
    virtual bool Init() { return Init(RSA_PKCS_V21, POLARSSL_MD_SHA256); }
    virtual void Clear();
	virtual bool Self_Test(bool verbose);
	virtual u32  Get_Revision(void);
	
	virtual bool Encrypt(void *p_rng, int mode, size_t ilen, u8 *input, u8 *output);
	virtual bool Decrypt(int mode, size_t *olen, u8 *input, u8 *output, size_t output_max_len);
	virtual bool Sign(void *p_rng, int mode, int hash_id, u32 hashlen, u8 *hash, u8 *sig);
	virtual bool CalHashAndSign(void *p_rng, int mode, int hash_id, u32 ilen, const u8 *input, u8 *sig);
	virtual bool Verify(int mode, int hash_id, u32 hashlen, const u8 *hash, const u8 *sig);
	virtual bool CalHashAndVerify(int mode, int hash_id, u32 ilen, u8 *input, u8 *sig);
	virtual bool Generate_Keypair(void *p_rng, u32 nbits, int exponent);
	virtual bool GetPublicKey(u8 *pKey, u32 *rSize);
	virtual bool GetPrivateKey(u8 *pKey, u32 *rSize);
	//virtual bool LoadTestKeys(void);
	// Takes P and Q prime factors. rSize = Size of prime factors
	virtual bool SetContext(const u8 *pP, const u8 *pQ, u32 Exponent, u32 rSize);
	virtual bool SetContextPublic(const u8 *pKey, u32 nBytes, u32 Exponent);
	virtual bool GetPrimes(u8 *pP, u8 *pQ, u32 *, u32 nSize);       // P and Q
	// Takes Public and Private keys. pN = Public and pD = Private, rSize = size of Keys
	virtual bool SetContextKeyPair(const u8 *pN, const u8 *pD, u32 rSize, u32 Exponent);

	virtual bool SetMode (int Padding, int Hash);
	virtual bool SetRandTestMode (bool bMode, u8 *pSeed, int nSeedLen);

    // x.509/PEM routines
    virtual bool SetContextPublicPEM(const char * pKey);
    virtual bool SetContextPrivatePEM(const char * pKey);
	virtual bool GetPublicKeyPEM(char *pKey, u32 *rSize);
	virtual bool GetPrivateKeyPEM(char *pKey, u32 *rSize);
    virtual bool GetRootCAPEM(const char * subject, uint32_t validity_days, char * PEM, u32 * rSize);
    virtual bool GetCertSignReqPEM(const char * subject, int key_usage, char * PEM, u32 * rSize);

    // sealed envelope routines
    virtual bool Generate_Keypair_Sealed(void * p_rng, u32 nbits, int exponent,
                                         const unsigned char * EKEY,        size_t   EKEY_len,
                                         const unsigned char * MKEY,        size_t   MKEY_len,
                                               unsigned char * ENV_PEM_prv, size_t * ENV_PEM_prv_len,   // NULLs to disable prv
                                               unsigned char * ENV_PEM_pub, size_t * ENV_PEM_pub_len,   // NULLs to disable pub
                                               unsigned char * ENV_PEM_csr, size_t * ENV_PEM_csr_len,   // NULLs to disable csr
                                                  const char * subject, int key_usage);                 // NULL/-1 to disable csr

    virtual bool SetContextPrivateSealed(const unsigned char * EKEY,           size_t   EKEY_len,
                                         const unsigned char * MKEY,           size_t   MKEY_len,
                                         const unsigned char * ENV_sealed_prv, size_t   ENV_sealed_prv_len);

    virtual bool SetContextPublicSealed (const unsigned char * EKEY,           size_t   EKEY_len,
                                         const unsigned char * MKEY,           size_t   MKEY_len,
                                         const unsigned char * ENV_sealed_pub, size_t   ENV_sealed_pub_len);

	virtual bool Restore_Device_Context(void);
		
private:	

    rsa_context ctx;

    // self-test avoid stack storage...
    u32 stime;
    u32 ETime[8];
};
