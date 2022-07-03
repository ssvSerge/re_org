/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_sha256.cpp 21519 2013-10-08 00:12:14Z spcorcoran $
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
/*
**    Includes
*/

#include "ICypher.h"

#include "c_sha256.h"

static char rid[] = "$Rev: 21519 $";

/******************************************************************************
** Revision
******************************************************************************/
u32 oSHA256::Get_Revision(void)
{
    int rev;
       sscanf( &rid[6], "%d", &rev );
    return (u32)rev;
}

/******************************************************************************
**    Generate a Message Digest using SHA-256 HASH or SHA-224
**    Host Function
******************************************************************************/
bool oSHA256::Hash(const u8 *Message, int NumBytes, u8 *HashValue, bool is224)
{
    if (!Message)
        return false;
    if (NumBytes < 1)
        return false;
    if (!HashValue)
        return false;
    /*
    **    Initialize
    */
    sha256_starts(&ctx, is224);

    /*
    **    Process Message (-1 is for NULL PAD)
    */
    sha256_update(&ctx, Message, NumBytes);

    /*
    **    Generate Hash value
    */
    sha256_finish(&ctx, hashval);

    uint HashLen = (is224) ? 28 : 32;
    memcpy(HashValue, hashval, HashLen);

    return true;
}
