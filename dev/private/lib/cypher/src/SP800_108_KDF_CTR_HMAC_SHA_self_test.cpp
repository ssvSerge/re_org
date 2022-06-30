#include <string.h>
#include "SP800_108_KDF_CTR_HMAC_SHA.h"
#include "AutoHeapBuffer.h"
#include "Platform.h"
#include <stdio.h>

static bool test_256()
{
    static uint8_t VERIFY_256[128] = { 0xB0,0x8C,0xB3,0x63,0x62,0x29,0x94,0x75,0xB4,0x82,0x5D,0x0A,0x78,0x04,0x94,0x46,
                                       0x08,0xBC,0xC9,0x08,0xD7,0x32,0x70,0xE7,0xBA,0x80,0xCA,0x41,0x0D,0xFE,0x0A,0xAF,
                                       0x02,0x9F,0x3A,0xFC,0x39,0xA3,0xA1,0xB5,0x82,0x4C,0x22,0x12,0x5D,0x7A,0x1F,0x43,
                                       0x50,0x21,0x16,0x07,0xB3,0x9B,0xB3,0xC5,0xF9,0x5C,0x7B,0x57,0x75,0x17,0x67,0x6A,
                                       0x45,0x01,0x2F,0xC7,0xE4,0x7C,0x58,0xFC,0xC2,0xEE,0xA5,0x94,0x6C,0x2E,0xCA,0x99,
                                       0x44,0x73,0xAE,0xEC,0x63,0x4B,0x15,0x16,0x67,0x57,0x5F,0x85,0x0A,0xF6,0xC7,0x09,
                                       0x63,0x21,0xE2,0x34,0xF1,0xA6,0x95,0x2D,0xCC,0x8C,0x4C,0x91,0x8A,0x7E,0x17,0x1C,
                                       0xF3,0xF3,0xD5,0x77,0x8C,0x79,0xA4,0xA4,0x6B,0xFE,0x42,0xD6,0xA1,0xE6,0x3C,0xDD };

    AutoHeapBuffer auto_Ki_host(256 / 8);
    if (!auto_Ki_host.u8Ptr())
        return false;
    AutoHeapBuffer auto_Ki_dev(256 / 8);
    if (!auto_Ki_dev.u8Ptr())
        return false;
    AutoHeapBuffer auto_Cert_host(1024);
    if (!auto_Cert_host.u8Ptr())
        return false;
    AutoHeapBuffer auto_Cert_dev(1024);
    if (!auto_Cert_dev.u8Ptr())
        return false;
    const char * Labels    = "ED2H" "EH2D" "MD2H" "MH2D";
    AutoHeapBuffer auto_Ko(1024 / 8);
    if (!auto_Ko.u8Ptr())
        return false;

    for (size_t i = 0; i < 256 / 8; i++)
    {
        auto_Ki_host.u8Ptr()[i] = (unsigned char) (i & 0xFF);
        auto_Ki_dev.u8Ptr()[i]  = (unsigned char) (i & 0xFF);
    }

    for (size_t i = 0; i < 1024; i++)
    {
        auto_Cert_host.u8Ptr()[i] = (unsigned char) (i & 0xFF);
        auto_Cert_dev.u8Ptr()[i]  = (unsigned char) (i & 0xFF);
    }

    int rc;

    // SUCCESS
    rc = SP800_108_KDF_CTR_HMAC_SHA(POLARSSL_MD_SHA256,
                                    auto_Ki_host.u8Ptr(),
                                    auto_Ki_host.Len(),
                                    auto_Ki_dev.u8Ptr(),
                                    auto_Ki_dev.Len(),
                                    auto_Cert_host.charPtr(),
                                    auto_Cert_host.Len(),
                                    auto_Cert_dev.charPtr(),
                                    auto_Cert_dev.Len(),
                                    reinterpret_cast<const unsigned char *>(Labels),
                                                                     strlen(Labels),
                                    auto_Ko.u8Ptr(),
                                    auto_Ko.Len());
    if (rc)
        return false;

#if 0
    fprintf(stderr, "static uint8_t VERIFY_256[128] = { ");
    for (size_t i = 0; i < auto_Ko.Len(); i++)
    {
        if (i)
            fprintf(stderr, ",");
        fprintf(stderr, "0x%02X", auto_Ko.u8Ptr()[i] & 0xFF);
    }
    fprintf(stderr, " };\n");
#endif

    if (auto_Ko.Len() != sizeof(VERIFY_256))
        return false;
    if (memcmp(auto_Ko.u8Ptr(), VERIFY_256, sizeof(VERIFY_256)))
        return false;
    return true;
}

static bool test_512()
{
    static uint8_t VERIFY_512[128] = { 0xC8,0x4D,0x73,0xBC,0x70,0xC5,0xB9,0x42,0x6C,0x7F,0xFC,0x0C,0x4C,0xC9,0x81,0x05,
                                       0xD9,0x36,0xCF,0xA0,0xDF,0x45,0xE5,0x2D,0xEF,0xAE,0xA1,0x56,0xB9,0xB9,0xC4,0x86,
                                       0x26,0x90,0xA7,0x51,0x7E,0x40,0x65,0xE3,0x9C,0x5B,0x60,0xDC,0xA9,0x76,0x6D,0xFF,
                                       0x6C,0x91,0xA1,0x36,0x14,0x9F,0x99,0xED,0x83,0x86,0xAE,0x1D,0x16,0x6E,0xDE,0x13,
                                       0xF9,0xAC,0x15,0x50,0x74,0x8E,0xCC,0xF0,0x79,0x63,0x8E,0x0E,0x73,0x96,0x60,0xC8,
                                       0xD2,0x29,0xD7,0x31,0xB7,0x4E,0xF1,0xC8,0x92,0xF6,0x70,0x17,0x22,0xFB,0xD6,0xE7,
                                       0x06,0x0E,0x07,0x9E,0xB5,0x62,0x2C,0x0D,0x41,0x0D,0xF7,0xA1,0x9F,0x75,0x22,0x3D,
                                       0xD6,0xFE,0xA0,0x7F,0xE3,0x57,0xAD,0xF2,0x1A,0x56,0x35,0xD3,0x31,0x64,0x9D,0x9F };

    AutoHeapBuffer auto_Ki_host(512 / 8);
    if (!auto_Ki_host.u8Ptr())
        return false;
    AutoHeapBuffer auto_Ki_dev(512 / 8);
    if (!auto_Ki_dev.u8Ptr())
        return false;
    AutoHeapBuffer auto_Cert_host(1024);
    if (!auto_Cert_host.u8Ptr())
        return false;
    AutoHeapBuffer auto_Cert_dev(1024);
    if (!auto_Cert_dev.u8Ptr())
        return false;
    const char * Labels    = "ED2H" "EH2D" "MD2H" "MH2D";
    AutoHeapBuffer auto_Ko(1024 / 8);
    if (!auto_Ko.u8Ptr())
        return false;

    for (size_t i = 0; i < 512 / 8; i++)
    {
        auto_Ki_host.u8Ptr()[i] = (unsigned char) (i & 0xFF);
        auto_Ki_dev.u8Ptr()[i]  = (unsigned char) (i & 0xFF);
    }

    for (size_t i = 0; i < 1024; i++)
    {
        auto_Cert_host.u8Ptr()[i] = (unsigned char) (i & 0xFF);
        auto_Cert_dev.u8Ptr()[i]  = (unsigned char) (i & 0xFF);
    }

    int rc;

    // SUCCESS
    rc = SP800_108_KDF_CTR_HMAC_SHA(POLARSSL_MD_SHA512,
                                    auto_Ki_host.u8Ptr(),
                                    auto_Ki_host.Len(),
                                    auto_Ki_dev.u8Ptr(),
                                    auto_Ki_dev.Len(),
                                    auto_Cert_host.charPtr(),
                                    auto_Cert_host.Len(),
                                    auto_Cert_dev.charPtr(),
                                    auto_Cert_dev.Len(),
                                    reinterpret_cast<const unsigned char *>(Labels),
                                                                     strlen(Labels),
                                    auto_Ko.u8Ptr(),
                                    auto_Ko.Len());
    if (rc)
        return false;

#if 0
    fprintf(stderr, "static uint8_t VERIFY_512[128] = { ");
    for (size_t i = 0; i < auto_Ko.Len(); i++)
    {
        if (i)
            fprintf(stderr, ",");
        fprintf(stderr, "0x%02X", auto_Ko.u8Ptr()[i] & 0xFF);
    }
    fprintf(stderr, " };\n");
#endif

    if (auto_Ko.Len() != sizeof(VERIFY_512))
        return false;
    if (memcmp(auto_Ko.u8Ptr(), VERIFY_512, sizeof(VERIFY_512)))
        return false;
    return true;
}

bool SP800_108_KDF_CTR_HMAC_SHA_self_test()
{
    if (!test_256())
        return false;
    if (!test_512())
        return false;
    return true;
}
