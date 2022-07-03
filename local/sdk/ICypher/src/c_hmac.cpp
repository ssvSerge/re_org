/*******************************************************************************
**
**  V300b Updated Board Support Package
**  $Id: v300b_hmac.cpp 22877 2014-03-03 03:38:32Z spcorcoran $
**
**  COPYRIGHT INFORMATION:
**      This software is proprietary and confidential.
**      By using this software you agree to the terms and conditions of the
**      associated Lumidigm Inc. License Agreement.
**
**      (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
**  Keyed-Hash Message Authentication Code (HMAC)
**  FIPS PUB 198, 198-1
*******************************************************************************/
/*
**  Includes
*/

#include "ICypher.h"
#include "Auto_Crypto_Objects.h"

#include <iostream>
#include <fstream>

#include "c_hmac.h"
//#include "c_hmac_tvec.h"  // Test Vectors

static char rid[] = "$Rev: 22877 $";

/******************************************************************************
** Revision
******************************************************************************/
u32 oHMAC::Get_Revision(void)
{
    int rev;
    sscanf( &rid[6], "%d", &rev );
    return (u32)rev;
}

/******************************************************************************
** Initialization
******************************************************************************/
bool oHMAC::Init( void )
{
    return true;
}//end

/******************************************************************************
** Keyed-Hash Message Authentication Code (HMAC)
******************************************************************************/
bool oHMAC::HMAC( u8 *Msg, int MsgSize, u8 *Key, int KeySize, u8 *MAC, int *MACSize, int Mode)
{
    if (!Msg)
        return false;
    if (MsgSize < 1)
        return false;
    if (!Key)
        return false;
    #if defined(HMAC_KEY_LENGTH_ENFORCE)
        // HMAC is subject to the algorithm and key size transitions described in NIST SP 800-131A.
        // HMAC Generation with Key Size < 112 bits (i.e., < 14 bytes) is Disallowed as of January 1, 2014.
        if (KeySize < 14)
            return false;
    #endif
    if (!MAC)
        return false;

    AutoHeapBuffer Auto_HMAC(POLARSSL_MD_MAX_SIZE);

    switch( Mode )
    {
        case SHA1_MODE:
        {
            if (*MACSize > 20)
                *MACSize = 20;
            Auto_sha1_context ctx;
            if (ctx.Ptr() == NULL)
                return false;
            if (sha1_hmac_starts(ctx.Ptr(), Key, KeySize) != 0)
                return false;
            sha1_hmac_update(ctx.Ptr(), Msg, MsgSize);
            sha1_hmac_finish(ctx.Ptr(), Auto_HMAC.u8Ptr());
            memcpy(MAC, Auto_HMAC.u8Ptr(), *MACSize);
            break;
        }
        case SHA224_MODE:
        {
            if (*MACSize > 28)
                *MACSize = 28;
            Auto_sha256_context ctx;
            if (sha256_hmac_starts(ctx.Ptr(), Key, KeySize, 1/*is224*/) != 0)
                return false;
            sha256_hmac_update(ctx.Ptr(), Msg, MsgSize);
            sha256_hmac_finish(ctx.Ptr(), Auto_HMAC.u8Ptr());
            memcpy(MAC, Auto_HMAC.u8Ptr(), *MACSize);
            break;
        }
        case SHA256_MODE:
        {
            if (*MACSize > 32)
                *MACSize = 32;
            Auto_sha256_context ctx;
            if (sha256_hmac_starts(ctx.Ptr(), Key, KeySize, 0/*is224*/) != 0)
                return false;
            sha256_hmac_update(ctx.Ptr(), Msg, MsgSize);
            sha256_hmac_finish(ctx.Ptr(), Auto_HMAC.u8Ptr());
            memcpy(MAC, Auto_HMAC.u8Ptr(), *MACSize);
            break;
        }
        case SHA384_MODE:
        {
            if (*MACSize > 48)
                *MACSize = 48;
            Auto_sha512_context ctx;
            if (sha512_hmac_starts(ctx.Ptr(), Key, KeySize, 1/*is384*/) != 0)
                return false;
            sha512_hmac_update(ctx.Ptr(), Msg, MsgSize);
            sha512_hmac_finish(ctx.Ptr(), Auto_HMAC.u8Ptr());
            memcpy(MAC, Auto_HMAC.u8Ptr(), *MACSize);
            break;
        }
        case SHA512_MODE:
        {
            if (*MACSize > 64)
                *MACSize = 64;
            Auto_sha512_context ctx;
            if (sha512_hmac_starts(ctx.Ptr(), Key, KeySize, 0/*is384*/) != 0)
                return false;
            sha512_hmac_update(ctx.Ptr(), Msg, MsgSize);
            sha512_hmac_finish(ctx.Ptr(), Auto_HMAC.u8Ptr());
            memcpy(MAC, Auto_HMAC.u8Ptr(), *MACSize);
            break;
        }
#if 0
        case SHA512_224_MODE:
            L = L_SHA512_224;
            B = B_SHA512_224;
            break;
        case SHA512_256_MODE:
            L = L_SHA512_256;
            B = B_SHA512_256;
            break;
#endif
        default:
            return false;
    }
    return true;
}//end


#if  0
/******************************************************************************
**  Entity Authentication Module Based on Dedicated Secret Key
**  pHMAC...Host Computed Response to
**      pHostRNC
**      pSensorRNC
**      pOtherData
**  Location of KEY...............ATSHA or MAXQ
******************************************************************************/
bool oHMAC::HMAC_Authentication( u256 *pHMAC, u256 *pHostRNC, u256 *pSensorRNC, void *pOtherData, size_t nOtherSize)
{
    IATSHA  *pATSHA = ICypher::GetInstance()->GetATSHA();

    int nSize;
    int nBytes;

    AutoHeapBuffer Auto_rHMAC(sizeof(u256));
    AutoHeapBuffer Auto_pBuf(sizeof(u256) + sizeof(u256) + nOtherSize);
    AutoHeapBuffer Auto_scratch(sizeof(u256));
    u8 *rHMAC   = Auto_rHMAC.u8Ptr();
    u8 *pBuf    =  Auto_pBuf.u8Ptr();
    u8 *scratch = Auto_scratch.u8Ptr();
    if (rHMAC == NULL || pBuf == NULL || scratch == NULL)
        return false;

    /*
    **  Packet Digest
    */

    memcpy( pBuf, pHostRNC, sizeof(u256) );
    memcpy( &pBuf[sizeof(u256)], pSensorRNC, sizeof(u256) );

    if( nOtherSize != 0)
        memcpy( &pBuf[2*sizeof(u256)], pOtherData, nOtherSize );

    nBytes = sizeof(u256) + sizeof(u256) + int(nOtherSize);

    /*
    **  Get Authentication Key
    **
    **  NOTE: MAXQ RESERVED_0 is Actual key
    **      Temporary get from ATSHA
    */
#if 1
    /*
    ** Read Encrypted Slots
    */
    if(!pATSHA->ReadSlotEncrypted( SECURE_AUTHENTICATION_KEY, scratch ) )
        return false;

#else
    IMAXQ   *pMAXQ  = ICypher::GetInstance()->GetMAXQ();
    if (!pMAXQ->GetKey( (u256*)scratch, RESERVED_0) )
        return false;
#endif

    /*
    **  Generate Code
    */
    if( !HMAC( pBuf, nBytes, scratch, sizeof(u256), rHMAC, &nSize, SHA256_MODE) )
        return false;

    if ( memcmp( pHMAC, rHMAC, nSize ) != 0 )
        return false;

    /*
    ** Authentication Successful
    */
    return true;
}

/******************************************************************************
**  Entity Authentication Module Based on Dedicated Secret Key
**  Generic Input
**  Location of KEY...............ATSHA or MAXQ
**  pHMAC....HMAC HASH using secret key of pOtherDAta input
******************************************************************************/
bool oHMAC::HMAC_Authentication_Generic( u256 *pHMAC, void *pOtherData, size_t nOtherSize)
{
    IATSHA  *pATSHA = ICypher::GetInstance()->GetATSHA();
    int nSize;

    AutoHeapBuffer Auto_rHMAC(sizeof(u256));
    AutoHeapBuffer Auto_scratch(sizeof(u256));
    u8 *rHMAC =   Auto_rHMAC.u8Ptr();
    u8* scratch = Auto_scratch.u8Ptr();
    if (rHMAC == NULL || scratch == NULL)
        return false;

    /*
    **  Get Authentication Key
    **
    **  NOTE: MAXQ RESERVED_0 is Actual key
    **      Temporary get from ATSHA
    */
#if 1
    /*
    ** Read Encrypted Slots
    */
    if(!pATSHA->ReadSlotEncrypted( SECURE_AUTHENTICATION_KEY, scratch ) )
        return false;;

#else
    IMAXQ   *pMAXQ  = ICypher::GetInstance()->GetMAXQ();
    if (!pMAXQ->GetKey( (u256*)scratch, RESERVED_0) )
        return false;;
#endif

    /*
    **  Generate Code
    */
    if( !HMAC( (u8*)pOtherData, int(nOtherSize), scratch, sizeof(u256), rHMAC, &nSize, SHA256_MODE) )
        return false;;

    if ( memcmp( pHMAC, rHMAC, nSize ) != 0 )
        return false;;

    /*
    ** Authentication Successful
    */
    return true;
}
#endif
