/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_rand.cpp 22716 2014-02-06 23:53:54Z spcorcoran $
**
**	COPYRIGHT INFORMATION:	
**		This software is proprietary and confidential.  
**		By using this software you agree to the terms and conditions of the 
**		associated Lumidigm Inc. License Agreement.
**
**		(c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
**	FIPS MODE
**		SP 800- 90A  CTR_DRBG
** 		key length 256b
**		seed length 384b
*******************************************************************************/
/*
**	Includes
*/
#include <cassert>
#include "AutoHeapBuffer.h"
#include "ICypher.h"
#include "lumi_random.h"
#include "c_rand.h"
#include "polarssl/ctr_drbg.h"
    
static char rid[] = "$Rev: 22716 $";

extern "C" ctr_drbg_context g_ctr_drbg_ctx;

/******************************************************************************
** Revision
******************************************************************************/
u32 oRAND::Get_Revision(void)
{
	int rev = 0;
   	sscanf( &rid[6], "%d", &rev );
	return (u32)rev;
}
	
/******************************************************************************
** Initialization
******************************************************************************/
bool oRAND::Init(bool predictRes )
{
	lumi_random_init(NULL, 0);
	ctr_drbg_set_prediction_resistance(&g_ctr_drbg_ctx, static_cast<int>(predictRes));
    return true;
}//end

/******************************************************************************
** Generate a Random Number Blks of size CTR_DRBG_BLOCKSIZE\
** N = total number of Bytes Requested
******************************************************************************/
bool oRAND::Random( u8 *pData, int N )
{
    lumi_random(NULL, pData, N);
    return true;
}

/******************************************************************************
**	Collect Random Data for NIST KATests Evaluation
******************************************************************************/
bool oRAND::Generate_NIST(
                            bool                  bPredictRes,
                            const unsigned char * EntropyInput,
                            size_t                EntropyInput_len,
                            const unsigned char * EntropyInputPR_1,
                            size_t                EntropyInputPR_1_len,
                            const unsigned char * EntropyInputPR_2,
                            size_t                EntropyInputPR_2_len,
                            const unsigned char * Nonce,
                            size_t                Nonce_len,
                            const unsigned char * PersonalizationString,
                            size_t                PersonalizationString_len,
                            const unsigned char * AdditionalInput_1,
                            size_t                AdditionalInput_1_len,
                            const unsigned char * AdditionalInput_2,
                            size_t                AdditionalInput_2_len,
                            const unsigned char * EntropyInputReseed,
                            size_t                EntropyInputReseed_len,
                            const unsigned char * AdditionalInputReseed,
                            size_t                AdditionalInputReseed_len,
                                  unsigned char * Result,
                            size_t                Result_len
                         )
{
    int rc = -1;

    assert(EntropyInput);
    assert(Nonce);
    // here we assert that correct entropy entities exist for both paths through the code...
    if (bPredictRes)
    {
        // Non-Deterministic
        assert( EntropyInputPR_1   &&  EntropyInputPR_1_len);
        assert( EntropyInputPR_2   &&  EntropyInputPR_2_len);
        assert(!EntropyInputReseed_len);
    }
    else
    {
        // Deterministic
        assert(!EntropyInputPR_1_len);
        assert(!EntropyInputPR_2_len);
        assert( EntropyInputReseed &&  EntropyInputReseed_len);
    }
    assert(Result);
    assert(Result_len);

    //assert(nEntropyBytes       == 32);
    //assert(nNonceBytes         == 16);
    //assert(nEntropyBytes + nNonceBytes <= CTR_DRBG_SEEDLEN);

    //fprintf(stdout, "Generate_NIST: %d EI %zu EIP1 %zu EIP2 %zu N %zu PS %zu AI1 %zu AI2 %zu EIR %zu AIR %zu R %zu\n",
    //      bPredictRes, EntropyInput_len, EntropyInputPR_1_len, EntropyInputPR_2_len, Nonce_len,
    //      PersonalizationString_len, AdditionalInput_1_len, AdditionalInput_2_len, EntropyInputReseed_len,
    //      AdditionalInputReseed_len, Result_len);

    lumi_random_init(EntropyInput, EntropyInput_len);

    // set up initial Nonce + PersonalizationString block to be used as "custom" during DRBG init
    AutoHeapBuffer auto_NoncePS(1024);
    assert(auto_NoncePS.u8Ptr());
    size_t offset = 0;
    if (Nonce && Nonce_len)
    {
        memcpy(auto_NoncePS.u8Ptr() + offset, Nonce, Nonce_len);
        offset += Nonce_len;
    }
    if (PersonalizationString && PersonalizationString_len)
    {
        memcpy(auto_NoncePS.u8Ptr() + offset, PersonalizationString, PersonalizationString_len);
        offset += PersonalizationString_len;
    }

    {
        ctr_drbg_context ctx;
        rc = ctr_drbg_init_entropy_len(&ctx, lumi_random, NULL/*p_entropy*/, auto_NoncePS.u8Ptr(), offset, EntropyInput_len);
        if (rc == 0)
        {
            if (bPredictRes)
                ctr_drbg_set_prediction_resistance(&ctx, CTR_DRBG_PR_ON);
            else
            {
                //fprintf(stdout, "RESEED\n");
                lumi_random_init(EntropyInputReseed, EntropyInputReseed_len);
                if (AdditionalInputReseed_len == 0)
                    AdditionalInputReseed = NULL;
                rc = ctr_drbg_reseed(&ctx, AdditionalInputReseed, AdditionalInputReseed_len);
            }
            if (rc == 0)
            {
                //fprintf(stdout, "GENERATE 1 ");
                if (bPredictRes)
                    lumi_random_init(EntropyInputPR_1, EntropyInputPR_1_len);
                rc = ctr_drbg_random_with_add(&ctx,
                                              Result, Result_len,
                                              const_cast<unsigned char *>(AdditionalInput_1), AdditionalInput_1_len);
                if (rc == 0)
                {
                    //fprintf(stdout, "GENERATE 2 ");
                    if (bPredictRes)
                        lumi_random_init(EntropyInputPR_2, EntropyInputPR_2_len);
                    rc = ctr_drbg_random_with_add(&ctx,
                                                  Result, Result_len,
                                                  const_cast<unsigned char *>(AdditionalInput_2), AdditionalInput_2_len);
                    if (rc == 0)
                    {
                        // NADA...
                    }
                }
            }
        }
        ctr_drbg_free(&ctx);
    }

    return (rc == 0);
}
