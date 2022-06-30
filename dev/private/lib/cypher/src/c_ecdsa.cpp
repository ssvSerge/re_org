
/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: c_ecdsa.cpp 28352 2016-02-19 03:38:32Z sillendu $
**
**	COPYRIGHT INFORMATION:
**		This software is proprietary and confidential.
**		By using this software you agree to the terms and conditions of the
**		associated Lumidigm Inc. License Agreement.
**
**		(c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
/*
**	Includes
*/

#include "ICypher.h"
#include "CypherTypes.h"
#include "c_ecdsa.h"
#include "c_sha1.h"
#include "c_sha256.h"
#include "c_envelope.h"
#include "lumi_random.h"
#include "Auto_Crypto_Objects.h"


static char rid[] = "$Rev: 28352 $";
/******************************************************************************
**
******************************************************************************/
oECDSA::oECDSA()
{
	memset(&ctx, 0, sizeof(ctx));
	Init();
	SetRandTestMode(false, NULL, 0);
}

/******************************************************************************
**
******************************************************************************/
oECDSA::~oECDSA()
{
	Clear();
}

/******************************************************************************
** Revision
******************************************************************************/
u32 oECDSA::Get_Revision(void)
{
	int rev;
	sscanf(&rid[6], "%d", &rev);
	return (u32)rev;
}

/******************************************************************************
** Initialize an ECDSA context
******************************************************************************/
bool oECDSA::Init(int ecp_gid, int hash_id)
{
	return SetMode(ecp_gid, hash_id);
}

/******************************************************************************
** Clear an ECDSA context
******************************************************************************/
void oECDSA::Clear()
{
	ecdsa_free(&ctx);
	SetRandTestMode(false, NULL, 0);
}

/******************************************************************************
** Initialize Test Mode, This permits disabling RNG and using known SEEDS to
**	generate known answer results with ECDSA Sign, KeyGen.  Test mode should be
**	off for all security related deployments and ONLY enabled for Test Vector
**	Qualification Tests
******************************************************************************/
bool oECDSA::SetRandTestMode(bool bMode, u8 *pSeed, int nSeedLen)
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
** Initialize an ECDSA context
******************************************************************************/
bool oECDSA::SetMode(int ecp_gid, int HashType)
{	
	ecdsa_free(&ctx);
	ecdsa_init(&ctx);
	return true;
}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::Generate_Keypair(void *p_rng, int ecp_gid, u32 nbits)
{
	int rc = -1;

	Clear();
	Init();

	SetRandTestMode(p_rng ? true : false, reinterpret_cast<u8*>(p_rng), nbits / 8);
	rc = ecdsa_genkey(&ctx, (ecp_group_id)ecp_gid, lumi_random, NULL);

	return rc == 0;
}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::GetPrivateKey(u8 *pD, u32 *rSize)
{
	size_t nBytes = (ctx.grp.nbits + 7) / 8;

	if (mpi_write_binary(&ctx.d, pD, nBytes) != 0) return false;	// export
	*rSize = (u32)nBytes;

	return true;

}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::GetPublicKey(u8 *pQx, u8*pQy, u32 *rSize)
{
	
	size_t nBytes = (ctx.grp.nbits + 7) / 8;

	if (mpi_write_binary(&ctx.Q.X, pQx, nBytes) != 0) return false;	// export
	if (mpi_write_binary(&ctx.Q.Y, pQy, nBytes) != 0) return false;	// export
	*rSize = (u32)nBytes;

	return true;

}

/******************************************************************************
**  Load the Context
**	Must have d, Qx, Qy
******************************************************************************/
bool oECDSA::SetContextKeyPair(int ecp_gid, const u8 *pQx, const u8  *pQy, const u8 *pD, u32 rSize)
{
	ecdsa_free(&ctx);
	ecdsa_init(&ctx);

	if (mpi_read_binary(&(ctx.Q.X), pQx, rSize) != 0 ||
		mpi_read_binary(&(ctx.Q.Y), pQy, rSize) != 0 ||
		mpi_lset(&(ctx.Q.Z), 1) ||
		mpi_read_binary(&(ctx.d), pD, rSize) !=0)
	{
		ecdsa_free(&ctx);
		return false;
	}

	if(ecp_use_known_dp(&ctx.grp,(ecp_group_id) ecp_gid) !=0)
	{
		ecdsa_free(&ctx);
		return false;
	}

	if (ecp_check_pubkey(&ctx.grp, &ctx.Q))
	{
		ecdsa_free(&ctx);
		return false;
	}


	if (ecp_check_privkey(&ctx.grp, &ctx.d))
	{
		ecdsa_free(&ctx);
		return false;
	}
	return true;
}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::SetContextPublic(int ecp_gid, const u8 *pQx, const u8  *pQy, u32 rSize)
{
	ecdsa_free(&ctx);
	ecdsa_init(&ctx);

	if (mpi_read_binary(&(ctx.Q.X), pQx, rSize) != 0 ||
		mpi_read_binary(&(ctx.Q.Y), pQy, rSize) != 0 ||
		mpi_lset(&(ctx.Q.Z), 1) )
	{
		ecdsa_free(&ctx);
		return false;
	}

	if (ecp_use_known_dp(&ctx.grp, (ecp_group_id)ecp_gid) != 0)
	{
		ecdsa_free(&ctx);
		return false;
	}

	if (ecp_check_pubkey(&ctx.grp, &ctx.Q))
	{
		ecdsa_free(&ctx);
		return false;
	}
	return true;

}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::Sign(void *p_rng, int hash_id, u32 hashlen, u8 *hash, u8 *r, u8* s)
{
	if (ecdsa_sign(&ctx.grp, &ctx.r, &ctx.s,
		&ctx.d, hash, hashlen, lumi_random, p_rng) != 0)
		return false;

	size_t nBytes = (ctx.grp.nbits + 7) / 8;
	if (mpi_write_binary(&ctx.r, r, nBytes) != 0) return false;
	if (mpi_write_binary(&ctx.s, s, nBytes) != 0) return false;

	return true;

}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::Verify(int hash_id, u32 hashlen, u8 *hash, u8 *r, u8* s)
{
	size_t len = (ctx.grp.nbits + 7) / 8;
	if (mpi_read_binary(&ctx.r, r, len) != 0) return false;	// export
	if (mpi_read_binary(&ctx.s, s, len) != 0) return false;	// export

	if (ecdsa_verify(&ctx.grp, hash, hashlen, &ctx.Q, &ctx.r, &ctx.s) != 0)
		return false;

	return true;
}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::CalHashAndSign(void *p_rng, int hash_id, u32 ilen, u8 *input, u8 *r, u8* s)
{
	AutoHeapBuffer Auto_hash(POLARSSL_MD_MAX_SIZE);
	if (Auto_hash.u8Ptr() == NULL)
		return false;

	u32 hashlen;
	switch (hash_id)
	{
		case SIG_ECDSA_SHA1:
		{
							 ISHA1 * pSHA1 = ICypher::GetInstance()->GetSHA1();
							 pSHA1->Hash(input, ilen, Auto_hash.u8Ptr());
							 hashlen = 160 / 8;
							 break;
		}
		case SIG_ECDSA_SHA224:
		{
							   ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
							   pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is224*/);
							   hashlen = 224 / 8;
							   break;
		}
		case SIG_ECDSA_SHA256:
		{
							   ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
							   pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is224*/);
							   hashlen = 256 / 8;
							   break;
		}
		case SIG_ECDSA_SHA384:
		{
							   ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
							   pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is384*/);
							   hashlen = 384 / 8;
							   break;
		}
		case SIG_ECDSA_SHA512:
		{
							   ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
							   pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is384*/);
							   hashlen = 512 / 8;
							   break;
		}
		default:
			return false;
	}

	bool bRC = Sign(p_rng, hash_id, hashlen, Auto_hash.u8Ptr(), r, s);
	return bRC;
}

/******************************************************************************
**
******************************************************************************/
bool oECDSA::CalHashAndVerify(int hash_id, u32 ilen, u8 *input, u8 *r, u8* s)
{
	AutoHeapBuffer Auto_hash(POLARSSL_MD_MAX_SIZE);
	if (Auto_hash.u8Ptr() == NULL)
		return false;

	u32 hashlen;
	switch (hash_id)
	{
		case SIG_ECDSA_SHA1:
		{
								ISHA1 * pSHA1 = ICypher::GetInstance()->GetSHA1();
								pSHA1->Hash(input, ilen, Auto_hash.u8Ptr());
								hashlen = 160 / 8;
								break;
		}
		case SIG_ECDSA_SHA224:
		{
								ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
								pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is224*/);
								hashlen = 224 / 8;
								break;
		}
		case SIG_ECDSA_SHA256:
		{
								ISHA256 * pSHA256 = ICypher::GetInstance()->GetSHA256();
								pSHA256->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is224*/);
								hashlen = 256 / 8;
								break;
		}
		case SIG_ECDSA_SHA384:
		{
								ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
								pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), true/*is384*/);
								hashlen = 384 / 8;
								break;
		}
		case SIG_ECDSA_SHA512:
		{
								ISHA512 * pSHA512 = ICypher::GetInstance()->GetSHA512();
								pSHA512->Hash(input, ilen, Auto_hash.u8Ptr(), false/*is384*/);
								hashlen = 512 / 8;
								break;
		}
		default:
			return false;
	}

	bool bRC = Verify(hash_id, hashlen, Auto_hash.u8Ptr(), r, s);
	return bRC;
}
