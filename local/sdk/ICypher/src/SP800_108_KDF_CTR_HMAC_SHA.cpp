#include <string.h>
#include "SP800_108_KDF_CTR_HMAC_SHA.h"
#include "AutoHeapBuffer.h"
#include "lumi_endian.h"
#include "polarssl/sha256.h"
#include "polarssl/sha512.h"

// --------------------------------------------------------------------------------------------------------------------------------
//
#define WRITE_PTR(out, in, len) \
    memcpy(out, in, len);       \
    out += len;

// returns zero on success
int SP800_108_KDF_CTR_HMAC_SHA(md_type_t             hashid,
                               const unsigned char * Ki_host,      // entropy from host
                               size_t                Ki_host_len,  
                               const unsigned char * Ki_dev,       // entropy from device
                               size_t                Ki_dev_len,
                               const          char * Cert_host,    // Host public cert in PEM format, used to create Context
                               size_t                Cert_host_len,
                               const          char * Cert_dev,     // Device public cert in PEM format, used to create Context
                               size_t                Cert_dev_len,
                               const unsigned char * Labels,
                               size_t                Labels_len,
                                     unsigned char * Ko,
                               size_t                Ko_len)
{
    // basic parameter assertions...

    size_t Ko_iter_len;             // size of output of SHAx
    if (hashid == POLARSSL_MD_SHA256)
        Ko_iter_len = 32;
    else if (hashid == POLARSSL_MD_SHA512)
        Ko_iter_len = 64;
    else
        return -1;

    if (!Ko)
        return -1;
    if (Ko_len < Ko_iter_len)
        return -1;
    if (Ko_len > SIZE_MAX / 8)      // be careful before we multiply
        return -1;
    if (Ko_len * 8 > 0xFFFF)
        return -1;

    size_t iters = Ko_len / Ko_iter_len;

    if (!Ki_host)
        return -1;
    if (Ki_host_len < Ko_iter_len)  // require minimum of SHAx-output-bits of host entropy
        return -1;
    if (!Ki_dev)
        return -1;
    if (Ki_dev_len < Ko_iter_len)   // require minimum of SHAx-output-bits of device entropy
        return -1;
    if (!Cert_host)
        return -1;
    if (Cert_host_len < 1024)       // RSA-2048 cert should be at least this size
        return -1;
    if (!Cert_dev)
        return -1;
    if (Cert_dev_len < 1024)        // RSA-2048 cert should be at least this size
        return -1;
    if (!Labels)
        return -1;
    if (Labels_len < iters)         // require at least one byte of labelling per iteration
        return -1;

    // we don't allow slop
    size_t iter_out_slop  = Ko_len % Ko_iter_len;
    if (iter_out_slop)
        return -1;

    // calculate step size over given labels
    size_t Labels_iter_len  = Labels_len / iters;
    size_t Labels_iter_slop = Labels_len % iters;
    // we don't allow slop
    if (Labels_iter_slop)
        return -1;

    int rc = -1;    // assume failure, prove otherwise

    // buffer to hold host + dev entropy in a single place - as this will be the HMAC-SHAx key
    size_t Ki_host_dev_len = Ki_host_len + Ki_dev_len;
    AutoHeapBuffer auto_Ki_host_dev(Ki_host_dev_len);
    unsigned char * Ki_host_dev = auto_Ki_host_dev.u8Ptr();
    if (Ki_host_dev)
    {
        // place entropy in buffer
        unsigned char * Ki_host_dev_cursor = Ki_host_dev;
        WRITE_PTR(Ki_host_dev_cursor, Ki_host, Ki_host_len);
        WRITE_PTR(Ki_host_dev_cursor, Ki_dev,  Ki_dev_len);

        // sizing and segmentation of Text that will be run through HMAC-SHAx
        uint32_t     counter        = 1;
        uint32_t     counter_BE;
        const size_t counter_len    = sizeof(counter);
        const size_t counter_offset = 0;

        // never used because it immediately follows the counter...
        //const size_t Label_offset   = counter_len;
        const size_t Text_len       = counter_len + Labels_iter_len + 1/*0x00*/ +
                                      Cert_host_len + Cert_dev_len + sizeof(uint16_t)/*L*/;

        AutoHeapBuffer auto_Text(Text_len);
        unsigned char * Text = auto_Text.u8Ptr();
        if (Text)
        {
            // encode fixed portions text to run through HMAC-SHAx
            unsigned char * Text_cursor = Text;
            Text_cursor += counter_len;                             // insert counter later - loop variant
            Text_cursor += Labels_iter_len;                         // insert lable later   - loop variant
            WRITE_PTR(Text_cursor, "", 1);
            WRITE_PTR(Text_cursor, Cert_host, Cert_host_len);       // Context: Cert_host
            WRITE_PTR(Text_cursor, Cert_dev, Cert_dev_len);         // Context: Cert_dev
            uint16_t L_BE = Hton16(Ko_len);                         // L in 16-bit big-endian format
            WRITE_PTR(Text_cursor, &L_BE, sizeof(L_BE));
            if (Text_cursor - Text == static_cast<ssize_t>(Text_len))
            {
                const unsigned char * Labels_cursor = Labels;
                      unsigned char * Ko_cursor     = Ko;
                for (size_t iter = 0; iter < iters; iter++)
                {
                    // encode variable portions of Text to run through HMAC-SHAx
                    // counter and label are consecutive in the serialization
                    Text_cursor = Text + counter_offset;
                    counter_BE = Hton32(counter);                   // counter in 32-bit big-endian format
                    WRITE_PTR(Text_cursor, &counter_BE, sizeof(counter_BE));
                    WRITE_PTR(Text_cursor, Labels_cursor, Labels_iter_len);

                    if (hashid == POLARSSL_MD_SHA256)
                        rc = sha256_hmac(Ki_host_dev, Ki_host_dev_len, Text, Text_len, Ko_cursor, 0/*is224*/);
                    else if (hashid == POLARSSL_MD_SHA512)
                        rc = sha512_hmac(Ki_host_dev, Ki_host_dev_len, Text, Text_len, Ko_cursor, 0/*is384*/);
                    else
                        rc = -1;

                    if (rc)
                        break;

                    // advance
                    Ko_cursor     += Ko_iter_len;
                    Labels_cursor += Labels_iter_len;
                    counter++;
                }
            }
            else rc = -1;
        }
        else rc = -1;
    }
    else rc = -1;

    return -(rc != 0);
}
