//
// THIS FILE IS FROZEN FOR FIPS VALIDATION!
// NO CHANGES ALLOWED.
//

/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_des.cpp 22612 2014-01-25 14:18:56Z spcorcoran $
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
#include "Auto_Crypto_Objects.h"

#define BLKSIZE 8
#include "c_des_init.h"
#include "c_des.h"

/******************************************************************************
**
******************************************************************************/
bool oDES::Self_Test( bool verbose )
{
    //int cnt = 0;

    AutoHeapBuffer Auto_tOut(16 * BLKSIZE);
    AutoHeapBuffer Auto_tIn (16 * BLKSIZE);
    AutoHeapBuffer Auto_IV(8);
    u8 *tOut = Auto_tOut.u8Ptr();
    u8 *tIn  = Auto_tIn.u8Ptr();
    u8 *pIV  = Auto_IV.u8Ptr();
    if ( (tOut==NULL) || (tIn==NULL) || (pIV == NULL) )
        return false;

    /*
    ** Test DES-ECB Single Block
    */
    //u32 stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT1[IV][0], 8);
    if (!Encrypt( &KAT1[PT][0], tOut, BLKSIZE, &KAT1[KEY1][0], NULL, NULL, pIV, DES_ECB ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, &KAT1[CT][0], BLKSIZE) != 0 )
        return false;

    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT1[IV][0], 8);
    if (!Decrypt( &KAT1[CT][0], tOut, BLKSIZE, &KAT1[KEY1][0], NULL, NULL, pIV, DES_ECB ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, &KAT1[PT][0], BLKSIZE) != 0 )
        return false;

    /*
    ** Test 3DES-ECB Single Block
    */
    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT2[IV][0], 8);
    if (!Encrypt( &KAT2[PT][0], tOut, BLKSIZE, &KAT2[KEY1][0], &KAT2[KEY2][0], &KAT2[KEY3][0], pIV, TDES_ECB ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, &KAT2[CT][0], sizeof(u64)) != 0 )
        return false;

    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT2[IV][0], 8);
    if (!Decrypt( &KAT2[CT][0], tOut, BLKSIZE, &KAT2[KEY1][0], &KAT2[KEY2][0], &KAT2[KEY3][0], pIV, TDES_ECB ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, &KAT2[PT][0], sizeof(u64)) != 0 )
        return false;

    /*
    ** Test 3DES-CBC Multi Block Message
    */
    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT3[IV][0], 8);
    if (!Encrypt( PT_MBM3, tOut, sizeof(PT_MBM3), &KAT3[KEY1][0], &KAT3[KEY2][0], &KAT3[KEY3][0], pIV, TDES_CBC ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, CT_MBM3, sizeof(CT_MBM3)) != 0 )
        return false;

    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT3[IV][0], 8);
    if (!Decrypt( CT_MBM3, tOut, sizeof(CT_MBM3), &KAT3[KEY1][0], &KAT3[KEY2][0], &KAT3[KEY3][0], pIV, TDES_CBC ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, PT_MBM3, sizeof(PT_MBM3)) != 0 )
        return false;

    /*
    ** Test 3DES-CBC Multi Block Message
    */
    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT4[IV][0], 8);
    if (!Encrypt( PT_MBM4, tOut, sizeof(PT_MBM4), &KAT4[KEY1][0], &KAT4[KEY2][0], &KAT4[KEY3][0], pIV, TDES_CBC ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, CT_MBM4, sizeof(CT_MBM4)) != 0 )
        return false;

    //stime = ICypher::GetInstance()->Timer_Start();
    memcpy(pIV, &KAT4[IV][0], 8);
    if (!Decrypt( CT_MBM4, tOut, sizeof(CT_MBM4), &KAT4[KEY1][0], &KAT4[KEY2][0], &KAT4[KEY3][0], pIV, TDES_CBC ) )
        return false;
    //ETime[cnt++] = ICypher::GetInstance()->Timer_Stop( stime );

    if ( memcmp( tOut, PT_MBM4, sizeof(PT_MBM4)) != 0 )
        return false;

    return true;
}//end
