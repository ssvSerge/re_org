/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_crypto.cpp 20311 2013-07-25 13:58:06Z spcorcoran $
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
#include "c_sha512_tvec.h"
#include "c_sha512.h"

/******************************************************************************
** Run Built-In Test
******************************************************************************/
bool oSHA512::Self_Test( bool verbose )
{
	//STime = ICypher::GetInstance()->Timer_Start();
    
    /*
    ** 384b KAT
    */
    if (!Hash(KAT2_MSG, sizeof(KAT2_MSG), hashval, 1/*is384*/))
        return false;
    if (memcmp(hashval, KAT2_MAC, sizeof(KAT2_MAC)) != 0)
    	return false;
    	
    /*
    ** 512b KAT
    */
    if (!Hash(KAT1_MSG, sizeof(KAT1_MSG), hashval, 0/*is384*/))
    	return false;
    if (memcmp(hashval, KAT1_MAC, sizeof(KAT1_MAC)) != 0)
    	return false;

	/*
	** 512b KAT HMAC_SHA-512    
   	*/
	if (!HMAC_Hash(KAT3_KEY, sizeof(KAT3_KEY), KAT3_MSG, sizeof(KAT3_MSG), hashval, 0/*is384*/))
		return false;
    if (memcmp(hashval, KAT3_MAC, sizeof(KAT3_MAC)) != 0)
    	return false;

    return true;
}
