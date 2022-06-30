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
#include "polarssl/ecdsa.h"

class oECDSA : public IECDSA, public MemoryBase
{
public:

	oECDSA();
	virtual ~oECDSA();

	virtual bool Init(int ecp_gid, int hash_id);
	virtual bool Init() { return Init(POLARSSL_ECP_DP_SECP256R1, POLARSSL_MD_SHA256); }
	virtual void Clear();
	virtual bool Self_Test(bool verbose);
	virtual u32  Get_Revision(void);

	virtual bool Generate_Keypair(void *p_rng, int ecp_gid, u32 nbits);
	virtual bool GetPrivateKey(u8 *pD, u32 *rSize);
	virtual bool GetPublicKey(u8 *pQx, u8*pQy, u32 *rSize);
	virtual bool SetContextKeyPair(int ecp_gid,  const u8 *pQx, const u8  *pQy, const u8 *pD, u32 rSize );
	virtual bool SetContextPublic(int ecp_gid, const u8 *pQx, const u8  *pQy, u32 rSize);

	virtual bool Sign(void *p_rng, int hash_id, u32 hashlen, u8 *hash, u8 *r, u8* s);
	virtual bool Verify(int hash_id, u32 hashlen, u8 *hash, u8 *r, u8* s);
	virtual bool CalHashAndSign(void *p_rng, int hash_id, u32 ilen, u8 *input, u8 *r, u8* s);
	virtual bool CalHashAndVerify(int hash_id, u32 ilen, u8 *input, u8 *r, u8* s);


	virtual bool SetMode(int ecp_gid, int Hash);
	virtual bool SetRandTestMode(bool bMode, u8 *pSeed, int nSeedLen);


private:

	ecdsa_context ctx;

};
