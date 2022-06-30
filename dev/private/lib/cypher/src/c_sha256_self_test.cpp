/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_sha256.cpp 21519 2013-10-08 00:12:14Z spcorcoran $
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

#include "c_sha256_tvec.h"
#include "c_sha256.h"

/******************************************************************************
** Init
******************************************************************************/
bool oSHA256::Self_Test( bool verbose )
{
	/*
	**	Known Answer Test #1
	*/	
	if( !Hash( KAT1_MSG, sizeof(KAT1_MSG), &hashval[0] ) )
		return false;
	
	if( memcmp( &hashval[0], KAT1_MAC, sizeof(KAT1_MAC) ) != 0 )
		return false;
	
	/*
	**	Known Answer Test #2
	*/	
	if( !Hash( KAT2_MSG, sizeof(KAT2_MSG), &hashval[0] ) )
		return false;
	
	if( memcmp( &hashval[0], KAT2_MAC, sizeof(KAT2_MAC) ) != 0 )
		return false;
		
	return true;
}
