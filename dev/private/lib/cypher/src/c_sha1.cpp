/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_sha1.cpp 21029 2013-09-06 22:16:59Z spcorcoran $
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
#include "c_sha1.h"

static char rid[] = "$Rev: 21029 $";

/******************************************************************************
** Revision
******************************************************************************/
u32 oSHA1::Get_Revision(void)
{
	int rev;
   	sscanf( &rid[6], "%d", &rev );
	return (u32)rev;
}

/******************************************************************************
**	Generate a Message Digest using SHA-256 HASH
**	Host Function
******************************************************************************/
bool oSHA1::Hash( const u8 *Message, int NumBytes, u8 *HashValue )
{
    if (!Message)
        return false;
    if (NumBytes < 1)
        return false;
    if (!HashValue)
        return false;
    sha1_starts(&ctx);
	sha1_update(&ctx, Message, NumBytes);
    sha1_finish(&ctx, HashValue);
	return true;
}
