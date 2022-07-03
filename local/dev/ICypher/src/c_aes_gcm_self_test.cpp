// ================================================================================================================================
//
//  ProtocolTester
//
//      Unit-test harness for the PKISCMsg protocol and related cryptographic transforms.
//
// ================================================================================================================================

#include <cstdio>

#include "ICypher.h"
#include "lumi_random.h"
#include "c_aes_gcm.h"
#include "IMemMgr.h"

// ================================================================================================================================
// SANG
#include "PlatformDev.h"
// SANG
// ================================================================================================================================

SECTION_SDRAM0_BANK1
const u8 cIV[16]     = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
SECTION_SDRAM0_BANK1
const u8 cP[16]      = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
SECTION_SDRAM0_BANK1
const u8 cA[16]      = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B };
SECTION_SDRAM0_BANK1
const u8 cC_xpct[16] = { 0xAA, 0xA9, 0x91, 0x9D, 0x9D, 0x87, 0xCF, 0xC4, 0xF9, 0xB1, 0x1D, 0x65, 0x88, 0x72, 0xA6, 0xF0 };
SECTION_SDRAM0_BANK1
const u8 cKey[16]    = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
SECTION_SDRAM0_BANK1
const u8 cT_xpct[16] = { 0x0D, 0x04, 0x50, 0x96, 0xAA, 0x13, 0x13, 0x1C, 0x0C, 0x40, 0x11, 0x2A, 0x12, 0x37, 0x19, 0x0A };


// ================================================================================================================================
// Synthetic minimal AES-128-GCM test vector without any external reference.
bool AES_GCM_Lumi_Test_1()
{
    // Nope, not gonna check if we can allocate 16 bytes.  Sue me, its a test, and this is tedious. AAL
    u8* IV     = (u8*)MALLOC(16); memcpy(IV,cIV,16);
    u8* P      = (u8*)MALLOC(16); memcpy(P,cP,16);
    u8* A      = (u8*)MALLOC(16); memcpy(A,cA,16);
    u8* C_xpct = (u8*)MALLOC(16); memcpy(C_xpct,cC_xpct,16);
    u8* Key    = (u8*)MALLOC(16); memcpy(Key,cKey,16);
    u8* T_xpct = (u8*)MALLOC(16); memcpy(T_xpct,cT_xpct,16);

    u8* C  = (u8*)MALLOC(16);
    u8* P2 = (u8*)MALLOC(16);
    u8* T  = (u8*)MALLOC(16);

    bool bRC;

    lumi_random_init(IV, 16);
    oAESGCM gcm;
    bRC = gcm.AuthenticatedEncrypt(P, A, 16, C, 16, Key, 16,
                                   IV, 16, T, 16);
    if (bRC)
    {
        bRC = memcmp(C, C_xpct, sizeof(cC_xpct)) == 0;
        if (bRC)
        {
            bRC = memcmp(T, T_xpct, sizeof(cT_xpct)) == 0;
            if (bRC)
            {
                bRC = gcm.AuthenticatedDecrypt(C, A, sizeof(cA), P2, 16, Key, sizeof(cKey),
                                               IV, sizeof(cIV), T, 16);
                if (bRC)
                {
                    bRC = memcmp(P, P2, 16) == 0;
                    if (bRC)
                    {
                        fprintf(stderr, "%s: success.\n", __FUNCTION__);
                    }
                }
            }
        }
    }

    FREE(IV);
    FREE(P);
    FREE(A);
    FREE(C_xpct);
    FREE(Key);
    FREE(T_xpct);

    FREE(C);
    FREE(P2);
    FREE(T);

    lumi_random_init(NULL, 0);
    return bRC;
}

// ================================================================================================================================
// THIS TEST VECTOR FROM:
// MACsec GCM-AES Test Vectors
// April 11, 2011
// Provided for IEEE P802.1 Security Task Group
// considertaion by Karen Randall
bool AES_GCM_MACSEC_128_Test_1()
{
    // Why isn't the IV const?
    u8 IV[12]     = { 0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81, 0xB2, 0xC2, 0x84, 0x65 };

    SECTION_SDRAM0_BANK1
    const u8 P[48]      = { 0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
                      0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
                      0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x00, 0x02 };

    SECTION_SDRAM0_BANK1
    const u8 A[28]      = { 0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63, 0x7A, 0x0D, 0x46, 0xDF, 0x99, 0x8D, 0x88, 0xE5, 0x2E, 0x00,
                      0xB2, 0xC2, 0x84, 0x65, 0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81 };


    SECTION_SDRAM0_BANK1
    const u8 C_xpct[48] = { 0x70, 0x1A, 0xFA, 0x1C, 0xC0, 0x39, 0xC0, 0xD7, 0x65, 0x12, 0x8A, 0x66, 0x5D, 0xAB, 0x69, 0x24,
                      0x38, 0x99, 0xBF, 0x73, 0x18, 0xCC, 0xDC, 0x81, 0xC9, 0x93, 0x1D, 0xA1, 0x7F, 0xBE, 0x8E, 0xDD,
                      0x7D, 0x17, 0xCB, 0x8B, 0x4C, 0x26, 0xFC, 0x81, 0xE3, 0x28, 0x4F, 0x2B, 0x7F, 0xBA, 0x71, 0x3D };

    SECTION_SDRAM0_BANK1
    const u8 Key[16] = { 0xAD, 0x7A, 0x2B, 0xD0, 0x3E, 0xAC, 0x83, 0x5A, 0x6F, 0x62, 0x0F, 0xDC, 0xB5, 0x06, 0xB3, 0x45 };

    SECTION_SDRAM0_BANK1
    const u8 T_xpct[16] = { 0x4F, 0x8D, 0x55, 0xE7, 0xD3, 0xF0, 0x6F, 0xD5, 0xA1, 0x3C, 0x0C, 0x29, 0xB9, 0xD5, 0xB8, 0x80 };

    u8 C[48];
    u8 P2[48];
    u8 T[16];

    bool bRC;

    lumi_random_init(IV, sizeof(IV));
    oAESGCM gcm;
    bRC = gcm.AuthenticatedEncrypt(P, A, sizeof(A), C, sizeof(C), Key, sizeof(Key),
                                   IV, sizeof(IV), T, sizeof(T));
    if (bRC)
    {
        bRC = memcmp(C, C_xpct, sizeof(C_xpct)) == 0;
        if (bRC)
        {
            bRC = memcmp(T, T_xpct, sizeof(T_xpct)) == 0;
            if (bRC)
            {
                bRC = gcm.AuthenticatedDecrypt(C, A, sizeof(A), P2, sizeof(P), Key, sizeof(Key),
                                               IV, sizeof(IV), T, sizeof(T));
                if (bRC)
                {
                    bRC = memcmp(P, P2, sizeof(P2)) == 0;
                    if (bRC)
                    {
                        fprintf(stderr, "%s: success.\n", __FUNCTION__);
                    }
                }
            }
        }
    }

    lumi_random_init(NULL, 0);
    return bRC;
}
//
// ================================================================================================================================

// ================================================================================================================================
// THIS TEST VECTOR FROM:
// MACsec GCM-AES Test Vectors
// April 11, 2011
// Provided for IEEE P802.1 Security Task Group
// considertaion by Karen Randall
bool AES_GCM_MACSEC_256_Test_1()
{
    u8 IV[12]     = { 0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81, 0xB2, 0xC2, 0x84, 0x65 };
    const u8 P[48]      = { 0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
                      0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
                      0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x00, 0x02 };
    const u8 A[28] = { 0xD6, 0x09, 0xB1, 0xF0, 0x56, 0x63, 0x7A, 0x0D, 0x46, 0xDF, 0x99, 0x8D, 0x88, 0xE5, 0x2E, 0x00,
                      0xB2, 0xC2, 0x84, 0x65, 0x12, 0x15, 0x35, 0x24, 0xC0, 0x89, 0x5E, 0x81 };
    const u8 C_xpct[48] = { 0xE2, 0x00, 0x6E, 0xB4, 0x2F, 0x52, 0x77, 0x02, 0x2D, 0x9B, 0x19, 0x92, 0x5B, 0xC4, 0x19, 0xD7,
                      0xA5, 0x92, 0x66, 0x6C, 0x92, 0x5F, 0xE2, 0xEF, 0x71, 0x8E, 0xB4, 0xE3, 0x08, 0xEF, 0xEA, 0xA7,
                      0xC5, 0x27, 0x3B, 0x39, 0x41, 0x18, 0x86, 0x0A, 0x5B, 0xE2, 0xA9, 0x7F, 0x56, 0xAB, 0x78, 0x36 };
    const u8 Key[32] = { 0xE3, 0xC0, 0x8A, 0x8F, 0x06, 0xC6, 0xE3, 0xAD, 0x95, 0xA7, 0x05, 0x57, 0xB2, 0x3F, 0x75, 0x48,
                      0x3C, 0xE3, 0x30, 0x21, 0xA9, 0xC7, 0x2B, 0x70, 0x25, 0x66, 0x62, 0x04, 0xC6, 0x9C, 0x0B, 0x72 };
    const u8 T_xpct[16] = { 0x5C, 0xA5, 0x97, 0xCD, 0xBB, 0x3E, 0xDB, 0x8D, 0x1A, 0x11, 0x51, 0xEA, 0x0A, 0xF7, 0xB4, 0x36 };

    u8 C[48];
    u8 P2[48];
    u8 T[16];

    bool bRC;

    lumi_random_init(IV, sizeof(IV));
    oAESGCM gcm;
    bRC = gcm.AuthenticatedEncrypt(P, A, sizeof(A), C, sizeof(C), Key, sizeof(Key),
                                   IV, sizeof(IV), T, sizeof(T));
    if (bRC)
    {
        bRC = memcmp(C, C_xpct, sizeof(C_xpct)) == 0;
        if (bRC)
        {
            bRC = memcmp(T, T_xpct, sizeof(T_xpct)) == 0;
            if (bRC)
            {
                bRC = gcm.AuthenticatedDecrypt(C, A, sizeof(A), P2, sizeof(P), Key, sizeof(Key),
                                               IV, sizeof(IV), T, sizeof(T));
                if (bRC)
                {
                    bRC = memcmp(P, P2, sizeof(P2)) == 0;
                    if (bRC)
                    {
                        fprintf(stderr, "%s: success.\n", __FUNCTION__);
                    }
                }
            }
        }
    }

    lumi_random_init(NULL, 0);
    return bRC;
}
//
// ================================================================================================================================

// ================================================================================================================================
//
bool AES_GCM_self_test()
{
    bool bRC;
    bRC = AES_GCM_Lumi_Test_1();
    if (bRC)
    {
        bRC = AES_GCM_MACSEC_128_Test_1();
        if (bRC)
        {
            bRC = AES_GCM_MACSEC_256_Test_1();
            if (bRC)
            {
            }
        }
    }

    return bRC;
}
