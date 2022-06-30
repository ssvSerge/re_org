/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_rand.h 22716 2014-02-06 23:53:54Z spcorcoran $
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
#include "polarssl/aes.h"
#include "c_aes.h"
#include "IMemMgr.h"

class oRAND : public IRAND,  public MemoryBase
{
	public:
	
					 oRAND() { /*Init();*/  }
	    virtual      ~oRAND() {}
		virtual bool Init( bool predictRes = false);
		virtual bool Self_Test( bool verbose );
		virtual u32  Get_Revision(void);
		virtual bool Random( u8 *pData, int N );	// Generate Random Numbers

        virtual bool Generate_NIST(
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
                                  );

	private:
};
