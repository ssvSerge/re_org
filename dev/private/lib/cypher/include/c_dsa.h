/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_dsa.h 20311 2013-07-25 13:58:06Z spcorcoran $
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

class oDSA :  public MemoryBase
{
	public:
	
		virtual bool Init( void );
		virtual bool Self_Test(void);

		//virtual bool DSA( u8 *Msg, int MsgSize, u8 *Key, int KeySize, u8 *MAC, int *MACSize);
#if 0		
		virtual bool Generate_DS(void);
		virtual bool Validate_DS(void);
		virtual bool GenKeyPair( void );
		
#endif
		 
	private:
		
		bool GenerateDomainParameters( int L, int N );
		bool Hash( u8 *Msg, int Nbytes, u8 *hashval);
		bool Rand( u8 *rNum, int Nbytes);

	public:
	    oDSA();
	    ~oDSA();
};
