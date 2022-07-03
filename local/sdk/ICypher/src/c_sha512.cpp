/*******************************************************************************
**
**    V300b Updated Board Support Package
**    $Id: v300b_crypto.cpp 20311 2013-07-25 13:58:06Z spcorcoran $
**
**    COPYRIGHT INFORMATION:    
**        This software is proprietary and confidential.  
**        By using this software you agree to the terms and conditions of the 
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include "ICypher.h"
#include "c_sha512.h"

static char rid[] = "$Rev: 20311 $";

/******************************************************************************
** Revision
******************************************************************************/
u32 oSHA512::Get_Revision(void)
{
    int rev;
       sscanf( &rid[6], "%d", &rev );
    return (u32)rev;
}

/******************************************************************************
** Generate Hash Code for Buffer
**    SHA-384 or SHA-512 or SHA-512/224 or SHA-512/256
******************************************************************************/
bool oSHA512::Hash(const u8 *Message, int NumBytes, u8 *HashValue, int is384)
{
    if (!Message)
        return false;
    if (NumBytes < 1)
        return false;
    if (!HashValue)
        return false;
    sha512_starts(&ctx, is384);
    sha512_update(&ctx, Message, NumBytes);
    sha512_finish(&ctx, hashval);
    int HashLen = is384 ? 48 : 64;
    memcpy(HashValue, hashval, HashLen);
    return true;
}

/******************************************************************************
**  Generate Hash Code for Buffer
**    HMAC_SHA-384 or HMAC_SHA-512
******************************************************************************/
bool oSHA512::HMAC_Hash(const u8 *pKey, int nKeySize, const u8 *pMsg, int nMsgSize, u8 *HashValue, int is384)
{
    if (sha512_hmac_starts(&ctx, pKey, nKeySize, is384) != 0)
        return false;
    sha512_hmac_update(&ctx, pMsg, nMsgSize);
    sha512_hmac_finish(&ctx, hashval);
    int HashLen = is384 ? 48 : 64;
    memcpy(HashValue, hashval, HashLen);
    return true;
}

#if 0

// XXX - jbates - I am leaving this here as reference for possibly porting
//                SHA512/256 and SHA512/224 to PolarSSL's implementation.
/******************************************************************************
** SHA-512 context setup
******************************************************************************/
void oSHA512::sha4_starts(int shaType)
{
    ctx.total[0] = 0;
    ctx.total[1] = 0;
    switch (shaType)
    {
    case SHA512_224:
        /* SHA-512/224 */
        ctx.state[0] = UL64(0x8C3D37C819544DA2);
        ctx.state[1] = UL64(0x73E1996689DCD4D6);
        ctx.state[2] = UL64(0x1DFAB7AE32FF9C82);
        ctx.state[3] = UL64(0x679DD514582F9FCF);
        ctx.state[4] = UL64(0x0F6D2B697BD44DA8);
        ctx.state[5] = UL64(0x77E36F7304C48942);
        ctx.state[6] = UL64(0x3F9D85A86A1D36C8);
        ctx.state[7] = UL64(0x1112E6AD91D692A1);
        break;
    case SHA512_256:
        ctx.state[0] = UL64(0x22312194FC2BF72C);
        ctx.state[1] = UL64(0x9F555FA3C84C64C2);
        ctx.state[2] = UL64(0x2393B86B6F53B151);
        ctx.state[3] = UL64(0x963877195940EABD);
        ctx.state[4] = UL64(0x96283EE2A88EFFE3);
        ctx.state[5] = UL64(0xBE5E1E2553863992);
        ctx.state[6] = UL64(0x2B0199FC2C85B8AA);
        ctx.state[7] = UL64(0x0EB72DDC81C52CA2);
        break;
    case SHA384:
        /* SHA-384 */
        ctx.state[0] = UL64(0xCBBB9D5DC1059ED8);
        ctx.state[1] = UL64(0x629A292A367CD507);
        ctx.state[2] = UL64(0x9159015A3070DD17);
        ctx.state[3] = UL64(0x152FECD8F70E5939);
        ctx.state[4] = UL64(0x67332667FFC00B31);
        ctx.state[5] = UL64(0x8EB44A8768581511);
        ctx.state[6] = UL64(0xDB0C2E0D64F98FA7);
        ctx.state[7] = UL64(0x47B5481DBEFA4FA4);
        break;
    case SHA512:
    default:
        /* SHA-512 */
        ctx.state[0] = UL64(0x6A09E667F3BCC908);
        ctx.state[1] = UL64(0xBB67AE8584CAA73B);
        ctx.state[2] = UL64(0x3C6EF372FE94F82B);
        ctx.state[3] = UL64(0xA54FF53A5F1D36F1);
        ctx.state[4] = UL64(0x510E527FADE682D1);
        ctx.state[5] = UL64(0x9B05688C2B3E6C1F);
        ctx.state[6] = UL64(0x1F83D9ABFB41BD6B);
        ctx.state[7] = UL64(0x5BE0CD19137E2179);
    }


   ctx.shaType = shaType;
}

#endif
