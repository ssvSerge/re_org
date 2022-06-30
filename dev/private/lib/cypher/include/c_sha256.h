/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_sha256.h 21519 2013-10-08 00:12:14Z spcorcoran $
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
#include "polarssl/sha256.h"

class oSHA256 : public ISHA256, public MemoryBase
{
	public:
	
	                  oSHA256() {}
	    virtual      ~oSHA256() {}
		virtual bool Init(void) { return true; }
		virtual bool Self_Test(bool verbose);
		virtual u32  Get_Revision(void);
			
		virtual bool Hash(const u8 *Message, int NumBytes, u8 *HashValue, bool is224 = false);
		
	private:
	
		u8 hashval[32];
        sha256_context ctx;
};

