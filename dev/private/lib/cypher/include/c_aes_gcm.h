/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_aes.h 21029 2013-09-06 22:16:59Z spcorcoran $
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

class oAESGCM : public IAESGCM, public MemoryBase
{
	public:

	                  oAESGCM() {}
	    virtual      ~oAESGCM() {}
		virtual bool Init(void);
		virtual bool Self_Test(bool verbose);
		virtual u32  Get_Revision(void);
		
		virtual bool AuthenticatedEncrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                          const u8 *Key, int KeyLength, u8 *IV, int IV_len, u8 *MAC, int MAC_len);
		virtual bool AuthenticatedDecrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                          const u8 *Key, int KeyLength, u8 *IV, int IV_len, const u8 *MAC, int MAC_len);

	private:
};

extern bool AES_GCM_self_test();
