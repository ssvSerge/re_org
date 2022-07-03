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

#include "lumi_random.h"
#include "c_aes_gcm.h"
#include "polarssl/gcm.h"

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
u32 oAESGCM::Get_Revision(void)
{
    int rev;
    sscanf( &rid[6], "%d", &rev );
    return (u32)rev;
}

/******************************************************************************
**  Initialize Object
******************************************************************************/
bool oAESGCM::Init(void)
{
    return true;
}

/******************************************************************************
**  Authenticated Decryption
******************************************************************************/
bool oAESGCM::AuthenticatedDecrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                   const u8 *Key, int KeyLength, u8 *IV, int IV_len, const u8 *MAC, int MAC_len)
{
    if (!in)
        return false;
    if (AAD_len < 0)    // we do not require AAD, but don't send us negative lengths!
        return false;
    if (NBytes < 4)     // we require a minimum 32-bit text
        return false;
    if (!Key)
        return false;
    if (!IV)
        return false;
    if (IV_len < 12)    // we require a MINIMUM 96-bit IV - NO MAX
        return false;
    if (!MAC)
        return false;
    if (MAC_len < 12)   // we require a MINIMUM 96-bit tag
        return false;
    if (MAC_len > 16)   // we require a MAXIMUM 128-bit tag
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

    bool bRC;
    gcm_context gcm_ctx;
    bRC = gcm_init(&gcm_ctx, POLARSSL_CIPHER_ID_AES, Key, KeyLength) == 0;
    if (bRC)
    {
        bRC = gcm_auth_decrypt(&gcm_ctx, NBytes, IV, IV_len, AAD, AAD_len, MAC, MAC_len, in, out) == 0;
    }
    gcm_free(&gcm_ctx);

    return bRC;
}

/******************************************************************************
**  Authenticated Encryption
******************************************************************************/
bool oAESGCM::AuthenticatedEncrypt(const u8 *in, const u8* AAD, int AAD_len, u8 *out, int NBytes,
                                   const u8 *Key, int KeyLength, u8 *IV, int IV_len, u8 *MAC, int MAC_len)
{
    if (!in)
        return false;
    if (AAD_len < 0)    // we do not require AAD, but don't send us negative lengths!
        return false;
    if (NBytes < 4)     // we require a minimum 32-bit text
        return false;
    if (!Key)
        return false;
    if (!IV)
        return false;
    if (IV_len < 12)    // we require a MINIMUM 96-bit IV - NO MAX
        return false;
    if (!MAC)
        return false;
    if (MAC_len < 12)   // we require a MINIMUM 96-bit tag
        return false;
    if (MAC_len > 16)   // we require a MAXIMUM 128-bit tag
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

    // on encrypt - we are in control of IVs, per:
    // SP800 38D RBG-based Construction - 128 bits of "DIRECT RANDOM", 0 bits of FREE FIELD.
    if (lumi_random(NULL/*p_rng*/, IV, IV_len) != 0)
        return false;

    bool bRC;
    gcm_context gcm_ctx;
    bRC = gcm_init(&gcm_ctx, POLARSSL_CIPHER_ID_AES, Key, KeyLength) == 0;
    if (bRC)
    {
        bRC = gcm_crypt_and_tag(&gcm_ctx, GCM_ENCRYPT, NBytes, IV, IV_len, AAD, AAD_len, in, out, MAC_len, MAC) == 0;
    }
    gcm_free(&gcm_ctx);

    return bRC;
}

bool oAESGCM::Self_Test(bool verbose)
{
return false;
}
