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
//#include "c_des_init.h"
#include "c_des.h"

static char rid[] = "$Rev: 22612 $";

/******************************************************************************
** Revision
******************************************************************************/
u32 oDES::Get_Revision(void)
{
    int rev;
       sscanf( &rid[6], "%d", &rev );
    return (u32)rev;
}

/******************************************************************************
** Initialization
******************************************************************************/
bool oDES::Init( void )
{
    return true;

}//end

/******************************************************************************
** Encryption Function
**
**        pIn..........Input Data
**          pOut.........Encrypted Output
**        NBytes.......Number of Input Bytes Module BLKSIZE
**        KEY1,KEY2,KEY3,
**        IV...........Initialization VEctor
**        Mode.........Mode of Operation DES_Mode_Type
**
******************************************************************************/
bool oDES::Encrypt(const u8 *pIn, u8 *pOut, int NBytes, const u8 *pKEY1, const u8 *pKEY2, const u8 *pKEY3, u8 *IV, int Mode  )
{
    int nblks = NBytes / BLKSIZE;

    if ( NBytes % BLKSIZE != 0 )
        return false;

    // we are required for FIPS 140-2 two-key TDES to limit blocks encrypted with same key.
    // because of how this method is employed (without a context structure), we do not know
    // if we are using two-key DES, or many blocks may be encrypted at the app-layer.
    // thus, we shall limit the amount generated in a particular call to this number...
    // SEE: SP 800-131A, Section 2
    if (nblks > int32_t(1)<<20)     // LINE 171
        return false;

    switch (Mode )
    {
        case DES_ECB:        // DES Electronic Codebook
        {
            #if defined(NO_MULTIBLOCK_ECB_MODE)
                if (nblks > 1)
                    return false;
            #endif
            if (!pKEY1)
                return false;
            Auto_des_context auto_des_ctx;
            if (!auto_des_ctx.Ptr())
                return false;
            if (des_setkey_enc(auto_des_ctx.Ptr(), pKEY1) != 0)
                return false;
            for (int j = 0 ; j < nblks; j++, pIn += 8, pOut += 8)
            {
                if (des_crypt_ecb(auto_des_ctx.Ptr(), pIn, pOut) != 0)
                    return false;
            }
        } break;

        case DES_CBC:        // DES Cipher Block Chaining
        {
            if (!pKEY1)
                return false;
            Auto_des_context auto_des_ctx;
            if (!auto_des_ctx.Ptr())
                return false;
            AutoHeapBuffer Auto_IV(BLKSIZE);
            u8 *pIV  = Auto_IV.u8Ptr();
            if (pIV == NULL)
                return false;
            if(IV)
                memcpy(pIV, IV, BLKSIZE);
            else
                memset(pIV, 0, BLKSIZE);
            if (des_setkey_enc(auto_des_ctx.Ptr(), pKEY1) != 0)
                return false;
            if (des_crypt_cbc(auto_des_ctx.Ptr(), DES_ENCRYPT, NBytes, pIV, pIn, pOut) != 0)
                return false;
            // copy out IV
            if (IV)
                memcpy(IV, pIV, BLKSIZE);
        } break;

        case TDES_ECB:        // Triple DES Electronic Codebook
        {
            #if defined(NO_MULTIBLOCK_ECB_MODE)
                if (nblks > 1)
                    return false;
            #endif
            if (!pKEY1 || !pKEY2 || !pKEY3)
                return false;
            Auto_des3_context auto_des3_ctx;
            if (!auto_des3_ctx.Ptr())
                return false;
            AutoHeapBuffer auto_tmp(BLKSIZE * 5);
            if (!auto_tmp.u8Ptr())
                return false;
            u8 * pKey = auto_tmp.u8Ptr();
            u8 * pBlk = auto_tmp.u8Ptr() + BLKSIZE * 3;
            u8 * pIV  = auto_tmp.u8Ptr() + BLKSIZE * 4;
            memcpy(pKey,      pKEY1, 8);
            memcpy(pKey +  8, pKEY2, 8);
            memcpy(pKey + 16, pKEY3, 8);
            if (IV)
                memcpy(pIV, IV, BLKSIZE);
            else
                memset(pIV, 0, BLKSIZE);
            if (des3_set3key_enc(auto_des3_ctx.Ptr(), pKey) != 0)
                return false;
            // FIRST BLOCK - XOR-in IV
            memcpy(pBlk, pIn, 8);
            for (unsigned i = 0; i < BLKSIZE; i++)
                pBlk[i] ^= pIV[i];
            if (des3_crypt_ecb(auto_des3_ctx.Ptr(), pBlk, pOut) != 0)
                return false;
            // SUBSEQUENT BLOCKS - NO IV SPEC'd AT ALL!!!
            // XXX - JAB - This is NIST-specified behavior!  Unbelievable!  DO NOT USE!
            for (int j = 1; j < nblks; j++)
            {
                pIn += 8;
                pOut += 8;
                if (des3_crypt_ecb(auto_des3_ctx.Ptr(), pIn, pOut) != 0)
                    return false;
            }
        } break;

        case TDES_CBC:        // Triple DES CBC
        {
            if (!pKEY1 || !pKEY2 || !pKEY3)
                return false;
            Auto_des3_context auto_des3_ctx;
            if (!auto_des3_ctx.Ptr())
                return false;
            AutoHeapBuffer auto_tmp(BLKSIZE * 4);
            if (!auto_tmp.u8Ptr())
                return false;
            u8 * pKey = auto_tmp.u8Ptr();
            u8 * pIV  = auto_tmp.u8Ptr() + BLKSIZE * 3;
            memcpy(pKey,      pKEY1, 8);
            memcpy(pKey +  8, pKEY2, 8);
            memcpy(pKey + 16, pKEY3, 8);
            if (IV)
                memcpy(pIV, IV, BLKSIZE);
            else
                memset(pIV, 0, BLKSIZE);
            if (des3_set3key_enc(auto_des3_ctx.Ptr(), pKey)!= 0)
                return false;
            if (des3_crypt_cbc(auto_des3_ctx.Ptr(), DES_ENCRYPT, NBytes, pIV, pIn, pOut) != 0)
                return false;
            if (IV)
                memcpy(IV, pIV, BLKSIZE);
        }
        break;
    }

    return true;
}

/******************************************************************************
**
******************************************************************************/
bool oDES::Decrypt(const u8 *pIn, u8 *pOut, int NBytes, const u8 *pKEY1, const u8 *pKEY2, const u8 *pKEY3, u8 *IV, int Mode  )
{
    int nblks = NBytes / BLKSIZE;

    if ( NBytes % BLKSIZE != 0 )
        return false;

    // we are required for FIPS 140-2 two-key TDES to limit blocks encrypted with same key.
    // because of how this method is employed (without a context structure), we do not know
    // if we are using two-key DES, or many blocks may be encrypted at the app-layer.
    // thus, we shall limit the amount generated in a particular call to this number...
    // SEE: SP 800-131A, Section 2
    if (nblks > int32_t(1)<<20)     // LINE 171
        return false;

    switch (Mode )
    {
        case DES_ECB:        // DES Electronic Codebook
        {
            #if defined(NO_MULTIBLOCK_ECB_MODE)
                if (nblks > 1)
                    return false;
            #endif
            if (!pKEY1)
                return false;
            Auto_des_context auto_des_ctx;
            if (!auto_des_ctx.Ptr())
                return false;
            if (des_setkey_dec(auto_des_ctx.Ptr(), pKEY1) != 0)
                return false;
            for (int j = 0 ; j < nblks; j++, pIn += 8, pOut += 8)
            {
                if (des_crypt_ecb(auto_des_ctx.Ptr(), pIn, pOut) != 0)
                    return false;
            }
        } break;

        case DES_CBC:        // DES Cipher Block Chaining
        {
            if (!pKEY1)
                return false;
            Auto_des_context auto_des_ctx;
            if (!auto_des_ctx.Ptr())
                return false;
            AutoHeapBuffer Auto_IV(BLKSIZE);
            u8 *pIV  = Auto_IV.u8Ptr();
            if (pIV == NULL)
                return false;
            if(IV)
                memcpy(pIV, IV, BLKSIZE);
            else
                memset(pIV, 0, BLKSIZE);
            if (des_setkey_dec(auto_des_ctx.Ptr(), pKEY1) != 0)
                return false;
            if (des_crypt_cbc(auto_des_ctx.Ptr(), DES_ENCRYPT, NBytes, pIV, pIn, pOut) != 0)
                return false;
            if (IV)
                memcpy(IV, pIV, BLKSIZE);
        } break;

        case TDES_ECB:        // Triple DES Electronic Codebook
        {
            #if defined(NO_MULTIBLOCK_ECB_MODE)
                if (nblks > 1)
                    return false;
            #endif
            if (!pKEY1 || !pKEY2 || !pKEY3)
                return false;
            Auto_des3_context auto_des3_ctx;
            if (!auto_des3_ctx.Ptr())
                return false;
            AutoHeapBuffer auto_tmp(BLKSIZE * 5);
            if (!auto_tmp.u8Ptr())
                return false;
            u8 * pKey = auto_tmp.u8Ptr();
            u8 * pBlk = auto_tmp.u8Ptr() + BLKSIZE * 3;
            u8 * pIV  = auto_tmp.u8Ptr() + BLKSIZE * 4;
            memcpy(pKey,      pKEY1, 8);
            memcpy(pKey +  8, pKEY2, 8);
            memcpy(pKey + 16, pKEY3, 8);
            if (IV)
                memcpy(pIV, IV, BLKSIZE);
            else
                memset(pIV, 0, BLKSIZE);
            if (des3_set3key_dec(auto_des3_ctx.Ptr(), pKey) != 0)
                return false;
            // FIRST BLOCK - XOR-in IV
            memcpy(pBlk, pIn, 8);
            if (des3_crypt_ecb(auto_des3_ctx.Ptr(), pBlk, pOut) != 0)
                return false;
            for (unsigned i = 0; i < BLKSIZE; i++)
                pOut[i] ^= pIV[i];
            // SUBSEQUENT BLOCKS - NO IV SPEC'd AT ALL!!!
            // XXX - JAB - This is NIST-specified behavior!  Unbelievable!  DO NOT USE!
            for (int j = 1; j < nblks; j++)
            {
                pIn += 8;
                pOut += 8;
                if (des3_crypt_ecb(auto_des3_ctx.Ptr(), pIn, pOut) != 0)
                    return false;
            }
        } break;

        case TDES_CBC:        // Triple DES CBC
        {
            if (!pKEY1 || !pKEY2 || !pKEY3)
                return false;
            Auto_des3_context auto_des3_ctx;
            if (!auto_des3_ctx.Ptr())
                return false;
            AutoHeapBuffer auto_tmp(BLKSIZE * 4);
            if (!auto_tmp.u8Ptr())
                return false;
            u8 * pKey = auto_tmp.u8Ptr();
            u8 * pIV  = auto_tmp.u8Ptr() + BLKSIZE * 3;
            memcpy(pKey,      pKEY1, 8);
            memcpy(pKey +  8, pKEY2, 8);
            memcpy(pKey + 16, pKEY3, 8);
            if (IV)
                memcpy(pIV, IV, BLKSIZE);
            else
                memset(pIV, 0, BLKSIZE);
            if (des3_set3key_dec(auto_des3_ctx.Ptr(), pKey)!= 0)
                return false;
            if (des3_crypt_cbc(auto_des3_ctx.Ptr(), DES_DECRYPT, NBytes, pIV, pIn, pOut) != 0)
                return false;
            if (IV)
                memcpy(IV, pIV, BLKSIZE);
        }
        break;
    }

    return true;
}
