/*******************************************************************************
**
**  V300b Updated Board Support Package
**  $Id: v300b_aes.cpp 22612 2014-01-25 14:18:56Z spcorcoran $
**
**  COPYRIGHT INFORMATION:
**      This software is proprietary and confidential.
**      By using this software you agree to the terms and conditions of the
**      associated Lumidigm Inc. License Agreement.
**
**      (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
/*
**  Includes
*/
#include "ICypher.h"
#include "Auto_Crypto_Objects.h"

#include <iostream>
#include <fstream>

#include "c_aes_tvec.h" // Test Vectors
#include "c_aes.h"

#ifdef _VDSP
#include "blackfin.h"
#endif

#undef _DEBUG_

/******************************************************************************
**
**  Function:
**
**  Description:
**      Test AES Implementation by running KNOWN ANSWER Vectors
**
**  Inputs:
**
**  Returns:
**
**  Ensure:
**
******************************************************************************/
bool oAES::Self_Test( bool verbose )
{
    AutoHeapBuffer Auto_IV(16);
    AutoHeapBuffer Auto_cipher(256);
    AutoHeapBuffer Auto_text(256);
    u8 *pIV     = Auto_IV.u8Ptr();
    u8 *pCipher = Auto_cipher.u8Ptr();
    u8 *pText   = Auto_text.u8Ptr();
    if (pIV == NULL || pCipher == NULL || pText == NULL)
        return false;

    //  256b Test Single Block Encrypt
    memcpy(pIV, KAT1_IV, 16);
    if( !Encrypt( KAT1_PT, pCipher, sizeof(KAT1_PT), KAT1_KEY, sizeof(KAT1_KEY), pIV, AES_CBC ) )
        return false;

    if ( memcmp( KAT1_CT, pCipher, sizeof(KAT1_CT) ) != 0 )
        return false;

    // 256b Test Single Block Decrypt
    memcpy(pIV, KAT1_IV, 16);
    if(!Decrypt( KAT1_CT, pText, sizeof(KAT1_CT), KAT1_KEY, sizeof(KAT1_KEY), pIV, AES_CBC ))
        return false;

    if ( memcmp( KAT1_PT, pText, sizeof(KAT1_PT) ) != 0 )
        return false;

    //  256b Test Multi-Block Encrypt CBC
    memcpy(pIV, KAT2_IV, 16);
    if( !Encrypt( KAT2_PT, pCipher, sizeof(KAT2_PT), KAT2_KEY, sizeof(KAT2_KEY), pIV, AES_CBC ) )
        return false;

    if ( memcmp( KAT2_CT, pCipher, sizeof(KAT2_CT) ) != 0 )
        return false;

    //  128b Test Single Block Encrypt CBC
    memcpy(pIV, KAT3_IV, 16);
    if( !Encrypt( KAT3_PT, pCipher, sizeof(KAT3_PT), KAT3_KEY, sizeof(KAT3_KEY), pIV, AES_CBC ) )
        return false;

    if ( memcmp( KAT3_CT, pCipher, sizeof(KAT3_CT) ) != 0 )
        return false;

    //  128b Test Multi-Block Decrypt CBC
    memcpy(pIV, KAT4_IV, 16);
    if( !Decrypt( KAT4_CT, pText, sizeof(KAT4_CT), KAT4_KEY, sizeof(KAT4_KEY), pIV, AES_CBC ) )
        return false;

    if ( memcmp( KAT4_PT, pText, sizeof(KAT4_PT) ) != 0 )
        return false;

    //  192b Test Multi-Block Encrypt CBC
    memcpy(pIV, KAT5_IV, 16);
    if( !Encrypt( KAT5_PT, pCipher, sizeof(KAT5_PT), KAT5_KEY, sizeof(KAT5_KEY), pIV, AES_CBC ) )
        return false;

    if ( memcmp( KAT5_CT, pCipher, sizeof(KAT5_CT) ) != 0 )
        return false;

    //  192b Test Multi-Block Decrypt CBC
    memcpy(pIV, KAT5_IV, 16);
    if( !Decrypt( KAT5_CT, pText, sizeof(KAT5_CT), KAT5_KEY, sizeof(KAT5_KEY), pIV, AES_CBC ) )
        return false;

    if ( memcmp( KAT5_PT, pText, sizeof(KAT5_PT) ) != 0 )
        return false;

    //  256b Test Single Block Encrypt CTR
    memcpy(pIV, KAT6_IV, 16);
    if( !Encrypt( KAT6_PT, pCipher, sizeof(KAT6_PT), KAT6_KEY, sizeof(KAT6_KEY), pIV, AES_CTR ) )
        return false;

    if ( memcmp( KAT6_CT, pCipher, sizeof(KAT6_CT) ) != 0 )
        return false;

    //  256b Test Single Block Decrypt CTR
    memcpy(pIV, KAT6_IV, 16);
    if( !Decrypt( KAT6_CT, pText, sizeof(KAT6_CT), KAT6_KEY, sizeof(KAT6_KEY), pIV, AES_CTR ) )
        return false;

    if ( memcmp( KAT6_PT, pText, sizeof(KAT6_PT) ) != 0 )
        return false;

    //  256b GFSBox Known Answer Test
    for (int i = 0 ; i < 5 ; i++ )
    {
        memset( KAT_Key, 0, sizeof(u256) );     // Set Key
        memset( KAT_IV,  0, sizeof(u256) ) ;    // Set Initialization Vector

        if( !Encrypt( &GFSboxPT[i][0], KAT_Tmp, N_BLOCK, KAT_Key, sizeof(u256), KAT_IV, AES_CBC ) )
            return false;

        if( memcmp( &GFSboxCT[i][0], KAT_Tmp, N_BLOCK ) != 0 )
            return false;
    }

    //  256b KeySBox Known Answer Test
    for (int i = 0 ; i < 2 ; i++ )
    {
        memcpy( KAT_Key, &KeySboxKEY[i][0], sizeof(u256) );     // Set Key
        memset( KAT_IV,  0, sizeof(u256) ) ;    // Set Initialization Vector
        memset( KAT_PT,  0, sizeof(u256) ) ;    // Set PlainText Vector

        if( !Encrypt( KAT_PT, KAT_Tmp, N_BLOCK, KAT_Key, sizeof(u256), KAT_IV, AES_CBC ) )
            return false;

        if( memcmp( &KeySboxCT[i][0], KAT_Tmp, N_BLOCK ) != 0 )
            return false;
    }

    // 256b VarTxt Known Answer Test
    for (int i = 0 ; i < 5 ; i++ )
    {
        memset( KAT_Key, 0, sizeof(u256) );     // Set Key
        memset( KAT_IV,  0, sizeof(u256) ) ;    // Set Initialization Vector

        if( !Encrypt( &VarTxtPT[i][0], KAT_Tmp, N_BLOCK, KAT_Key, sizeof(u256), KAT_IV, AES_CBC ) )
            return false;

        if( memcmp( &VarTxtCT[i][0], KAT_Tmp, N_BLOCK ) != 0 )
            return false;
    }

    // 256b VarKey Known Answer Test
    for (int i = 0 ; i < 5 ; i++ )
    {
        memcpy( KAT_Key, &VarKey[i][0], sizeof(u256) ); // Set Key
        memset( KAT_IV,  0, sizeof(u256) ) ;            // Set Initialization Vector
        memset( KAT_PT,  0, sizeof(u256) ) ;            // Set PlainText Vector

        if( !Encrypt( KAT_PT, KAT_Tmp, N_BLOCK, KAT_Key, sizeof(u256), KAT_IV, AES_CBC ) )
            return false;

        if( memcmp( &VarKeyCT[i][0], KAT_Tmp, N_BLOCK ) != 0 )
            return false;
    }

    return true;
} //end
