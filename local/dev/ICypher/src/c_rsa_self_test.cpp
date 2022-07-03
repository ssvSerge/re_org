/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_rsa.cpp 22877 2014-03-03 03:38:32Z spcorcoran $
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
#include <time.h>
//#include <sys/time.h>

#include "ICypher.h"
#include "CypherTypes.h"
#include "c_rsa.h"
#include "c_rsa_tvec.h"
#include "Auto_Crypto_Objects.h"

/******************************************************************************
** Built-In Test
******************************************************************************/
bool oRSA::Self_Test(bool verbose)
{
    ISHA1    *pSHA1    = ICypher::GetInstance()->GetSHA1();
    size_t len;

    AutoHeapBuffer Auto_PT(PT_LEN);
    AutoHeapBuffer Auto_Buf(PT_LEN);
    AutoHeapBuffer Auto_CT(2048 / 8);
    AutoHeapBuffer Auto_hashval(20);
    if (Auto_PT.u8Ptr()      == NULL ||
        Auto_Buf.u8Ptr()     == NULL ||
        Auto_CT.u8Ptr()      == NULL ||
        Auto_hashval.u8Ptr() == NULL)
        return false;

    /*
    **    Test Big Numbers on Platform
    */
    if (verbose)
    {
        stime = ICypher::GetInstance()->Timer_Start();
        if (mpi_self_test(1/*verbose*/) != 0)
            return false;
        ETime[0] = ICypher::GetInstance()->Timer_Stop(stime);
    }

    if (!Generate_Keypair(NULL, 2048, 0x10001))
        return false;

    /*
    ** Validate Key Pairs
    */
    stime = ICypher::GetInstance()->Timer_Start();
    int rc = -1;
    rc = rsa_check_pubkey(&ctx);
    if (rc != 0)
        return false;
    rc = rsa_check_privkey(&ctx);
    if (rc != 0)
        return false;

    ETime[1] = ICypher::GetInstance()->Timer_Stop(stime);

    /*
    **    Load Plain Test for Test
    */
    memcpy(Auto_PT.u8Ptr(), RSA_PT, PT_LEN);

    /*
    **    Test Cipher Functions, Slow Only on Demand
    */
    if (verbose)
    {
        /*
        ** Currently, this is really slow.  Not going to run on every POR
        */
        stime = ICypher::GetInstance()->Timer_Start();
        if (!Encrypt(NULL, RSA_PUBLIC, PT_LEN, Auto_PT.u8Ptr(), Auto_CT.u8Ptr()))
             return false;
        ETime[2] = ICypher::GetInstance()->Timer_Stop(stime);

         stime = ICypher::GetInstance()->Timer_Start();
        if (!Decrypt(RSA_PRIVATE, &len, Auto_CT.u8Ptr(), Auto_Buf.u8Ptr(), PT_LEN))
             return false;
        ETime[3] = ICypher::GetInstance()->Timer_Stop(stime);

        if (memcmp(Auto_Buf.u8Ptr(), Auto_PT.u8Ptr(), len) != 0)
             return false;

         /*
         **    Test Signature Generation/Verification
         */
        if (!pSHA1->Hash(Auto_PT.u8Ptr(), PT_LEN, Auto_hashval.u8Ptr()))
             return false;

        stime = ICypher::GetInstance()->Timer_Start();
        if (!Sign(NULL, RSA_PRIVATE, SIG_RSA_SHA1, (u32)Auto_hashval.Len(), Auto_hashval.u8Ptr(), Auto_CT.u8Ptr()))
             return false;
        ETime[4] = ICypher::GetInstance()->Timer_Stop(stime);

        stime = ICypher::GetInstance()->Timer_Start();
        if (!Verify(RSA_PUBLIC, SIG_RSA_SHA1, (u32)Auto_hashval.Len(), Auto_hashval.u8Ptr(), Auto_CT.u8Ptr()))
             return false;
        ETime[5] = ICypher::GetInstance()->Timer_Stop(stime);

    }

     /*
     ** Cleanup
     */
    rsa_free(&ctx);

    if (verbose)
        fprintf(stdout, "oRSA::Self_Test passed.\n");

    return true;
}//end
