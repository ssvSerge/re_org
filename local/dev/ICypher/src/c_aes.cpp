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

#include "c_aes.h"
#include "aes_ecb.h"
#include "polarssl/aes.h"

#ifdef _VDSP
#include "blackfin.h"
#endif

#undef _DEBUG_

#ifndef __GNUC__
#pragma section("L1_data_a")
#endif

static char rid[] = "$Rev: 22612 $";

/******************************************************************************
** Revision
******************************************************************************/
u32 oAES::Get_Revision(void)
{
    int rev;
    sscanf( &rid[6], "%d", &rev );
    return (u32)rev;
}

/******************************************************************************
**  Initialize Object
******************************************************************************/
bool oAES::Init(void)
{
    return true;
}

/******************************************************************************
**  Decryption
******************************************************************************/
bool oAES::Decrypt(const u8 *in, u8 *out, int NBytes, const u8 *Key, int KeyLength, u8 *IV, int Mode )
{
    // CTR mode decrypt is the same as encrypt...
    if (Mode == AES_CTR)
        return Encrypt(in, out, NBytes, Key, KeyLength, IV, Mode);

    //  Inputs Must be PADDED to N_BLOCK modulo
    if ((NBytes % N_BLOCK) != 0)
        return false;

    // validate supported key length
    switch (KeyLength)
    {
        case 16:
            KeyLength = 128;
            break;
        case 24:
            KeyLength = 192;
            break;
        case 32:
            KeyLength = 256;
            break;
        case 128:
        case 192:
        case 256:
            break;
        default:
            return false;
    }
    // validate supported mode
    switch (Mode)
    {
        case AES_CBC:
        case AES_ECB:
            break;
        default:
            return false;
    }

    // get context struct from heap
    Auto_aes_context auto_ctx;
    AutoHeapBuffer Auto_iv(16);
    aes_context * ctx = auto_ctx.Ptr();
    unsigned char * iv = Auto_iv.u8Ptr();
    if (iv == NULL)
        return false;

    if (aes_setkey_dec(ctx, Key, KeyLength) != 0)
        return false;

    // IV supplied?
    if (IV)
        memcpy(iv, IV, N_BLOCK);
    else
        memset(iv, 0, 16);

    bool bRC = false;
    int length = NBytes;
    {
        #ifdef _VDSP
        //  Disable Interrupts
        u32 uiSaveInts = cli();
        #endif

        switch (Mode)
        {
            case AES_CBC:
                bRC = (aes_crypt_cbc(ctx, AES_DECRYPT, NBytes, iv, in, out) == 0);
                break;
            case AES_ECB:
                #if defined(NO_MULTIBLOCK_ECB_MODE)
                    if (length > N_BLOCK)
                        return false;
                #endif
                length = NBytes;
                while (length > 0)
                {

                    bRC = (aes_crypt_ecb(ctx, AES_DECRYPT, in, out) == 0);
                    if (bRC == false) {
                        break;
                    }
                    in += N_BLOCK;
                    out += N_BLOCK;
                    length -= N_BLOCK;
                }

                break;
        }

        #ifdef _VDSP
        // Enable Interrupts
        sti(uiSaveInts);
        #endif
    }

    // copy out IV
    if (IV)
        memcpy(IV, iv, N_BLOCK);

    return bRC;
}

/******************************************************************************
**
**  Function:  Encrypt()
**
**  Description:
**      AES Block Encryption with CBC
**
**  Inputs:
**      in..........plaintext
**      out.........ciphertext
**      NBlks.......Number of N_BLOCK's to encrypt
**
**  Returns:
**      true if Successfull
**
**  Ensure:
**
******************************************************************************/
bool oAES::Encrypt(const u8 *in, u8 *out, int NBytes, const u8 *Key, int KeyLength, u8 *IV, int Mode )
{
    //  Inputs Must be PADDED to N_BLOCK modulo
    if (Mode != AES_CTR && ((NBytes % N_BLOCK) != 0))
        return false;

    // validate supported key length
    switch (KeyLength)
    {
        case 16:
            KeyLength = 128;
            break;
        case 24:
            KeyLength = 192;
            break;
        case 32:
            KeyLength = 256;
            break;
        case 128:
        case 192:
        case 256:
            break;
        default:
            return false;
    }
    // validate supported mode
    switch (Mode)
    {
        case AES_CBC:
        case AES_ECB:
        case AES_CTR:
            break;
        default:
            return false;
    }

    // get context struct from heap
    Auto_aes_context auto_ctx;
    AutoHeapBuffer Auto_buffer(32);
    aes_context * ctx = auto_ctx.Ptr();
    unsigned char * buffer = Auto_buffer.u8Ptr();
    if (ctx == NULL || buffer == NULL)
        return false;
    unsigned char * stream_block = buffer;
    unsigned char * iv           = buffer + 16;

    // set key
    if (aes_setkey_enc(ctx, Key, KeyLength) != 0)
        return false;

    // IV supplied?
    if (IV)
        memcpy(iv, IV, N_BLOCK);
    else if (Mode == AES_CTR)   // no default zero IVs for AES-CTR
        return false;
    else
        memset(iv, 0, 16);

    bool bRC = false;
    int length = NBytes;
    size_t nc_off = 0;
    {
        #ifdef _VDSP
        //  Disable Interrupts
        u32 uiSaveInts = cli();
        #endif

        switch (Mode)
        {
            case AES_CBC:
                bRC = (aes_crypt_cbc(ctx, AES_ENCRYPT, NBytes, iv, in, out) == 0);
                break;
            case AES_ECB:
                #if defined(NO_MULTIBLOCK_ECB_MODE)
                    if (length > N_BLOCK)
                        return false;
                #endif
                length = NBytes;
                while (length > 0)
                {

                    bRC = (aes_crypt_ecb(ctx, AES_ENCRYPT, in, out) == 0);
                    if (bRC == false) {
                        break;
                    }
                    in += N_BLOCK;
                    out += N_BLOCK;
                    length -= N_BLOCK;
                }
                break;
            case AES_CTR:
                bRC = (aes_crypt_ctr(ctx, NBytes, &nc_off, iv, stream_block, in, out) == 0);
                break;
        }

        #ifdef _VDSP
        // Enable Interrupts
        sti(uiSaveInts);
        #endif
    }

    // copy out IV
    if (IV)
        memcpy(IV, iv, N_BLOCK);

    return bRC;
}
