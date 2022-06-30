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
#include "c_sha1_tvec.h"

/******************************************************************************
** Init
******************************************************************************/
bool oSHA1::Self_Test( bool verbose )
{
	if (!Hash(KAT1_MSG, sizeof(KAT1_MSG), hashval))
		return false;
	if( memcmp(hashval, KAT1_MAC, sizeof(hashval)) != 0 )
		return false;
    return true;
}
