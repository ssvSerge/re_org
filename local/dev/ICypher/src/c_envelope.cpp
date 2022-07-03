/*******************************************************************************
**
**    c_envelope - symmetric datagram envelope methods
**
**    COPYRIGHT INFORMATION:
**        This software is proprietary and confidential.
**        By using this software you agree to the terms and conditions of the
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include "c_envelope.h"
#include "envelope.h"
#include "envelope_aes_gcm.h"
#include "envelope_aes_hmac.h"
#include "lumi_random.h"
#include "AutoHeapBuffer.h"

#include <application/types_c.h>

// ================================================================================================================================
//
static bool Binary_Check_Length(uint32_t mid, size_t payload_len)
{
    switch (mid)
    {
        case IENVELOPE::HMAC_SHA_256_128_SECRET_KEY:
            return (payload_len == 16);
        case IENVELOPE::AES_GCM_256_SECRET_KEY:
        case IENVELOPE::AES_CTR_256_SECRET_KEY:
            return (payload_len == 32);
        case IENVELOPE::AES_GCM_192_SECRET_KEY:
        case IENVELOPE::AES_CTR_192_SECRET_KEY:
            return (payload_len == 24);
        case IENVELOPE::AES_GCM_128_SECRET_KEY:
        case IENVELOPE::AES_CTR_128_SECRET_KEY:
            return (payload_len == 16);
        case IENVELOPE::HMAC_SHA_256_FW_AUTH_TOKEN:
            return (payload_len == 80);// length =  _V100_ENC_FIRMWARE_HEADER + HMAC256. This has to be synced up with _V100_ENC_FIRMWARE_HEADER size
    }
    return false;
}

// ================================================================================================================================
//
static bool Is_PEM_Base64(const char * p, size_t l)
{
    // base64 min requirement + pointer safety
    if (l < 5)
        return false;

    // figure on whether we're dealing with MS-DOS carriage control...
    if (p[l - 1] != '\n')
        return false;
    size_t line_len = 65;
    bool msdos = false;
    if (p[l - 2] == '\r')
    {
        msdos = true;
        line_len = 66;
    }

    size_t lines    =  l / line_len;
    size_t partials = (l % line_len) > 0;
    lines += partials;
    const char * q = p;
    for (size_t line = 0; line < lines; line++)
    {
        if (l < 5)
            return false;
        size_t digits;
        if (line_len > l)               // last line?
            line_len = l;
        digits = line_len - 1;
        if (msdos)
            digits--;
        if (digits % 4 != 0)            // base64 comes 4 digits at a time
            return false;
        bool pad = false;
        for (size_t remain = digits; remain > 0; remain--, q++)
        {
            if (pad && *q != '=')
                return false;
            if (*q >= 'A' && *q <= 'Z') continue;
            if (*q >= 'a' && *q <= 'z') continue;
            if (*q >= '0' && *q <= '9') continue;
            if (*q == '+')              continue;
            if (*q == '/')              continue;
            if (*q == '=')              // base64 end-pad
            {
                pad = true;
                if (line != lines - 1)  // padding must be on last line only
                    return false;
                if (remain > 2)
                    return false;       // there can be no more than 2 pad characters
                continue;
            }
            return false;
        }
        if (msdos)
        {
            if (*q != '\r')
                return false;
            q++;
        }
        if (*q != '\n')
            return false;
        q++;
        l -= line_len;
    }
    return true;
}

// ================================================================================================================================
//
static bool Is_PEM(uint32_t mid, const char * payload, size_t payload_len)
{
    const char * BEGIN_DELIMITER = NULL;
    const char * END_DELIMITER   = NULL;

    switch (mid)
    {
        case IENVELOPE::RSA_2048_PUB_KEY:
            BEGIN_DELIMITER = "-----BEGIN PUBLIC KEY-----";
              END_DELIMITER = "-----END PUBLIC KEY-----";
            break;
        case IENVELOPE::RSA_2048_PRIV_KEY:
            BEGIN_DELIMITER = "-----BEGIN RSA PRIVATE KEY-----";
              END_DELIMITER = "-----END RSA PRIVATE KEY-----";
            break;
        case IENVELOPE::RSA_2048_CERT_REQ:
            BEGIN_DELIMITER = "-----BEGIN CERTIFICATE REQUEST-----";
              END_DELIMITER = "-----END CERTIFICATE REQUEST-----";
            break;
        case IENVELOPE::RSA_2048_CERT_CHAIN:
        case IENVELOPE::RSA_2048_LEAF_CERT:
        case IENVELOPE::RSA_2048_INTER_CA:
        case IENVELOPE::RSA_2048_ROOT_CA:
            BEGIN_DELIMITER = "-----BEGIN CERTIFICATE-----";
              END_DELIMITER = "-----END CERTIFICATE-----";
            break;
        default:
            return false;
    }

    bool rc = false;

    if (payload_len > 0)
    {
        size_t BEGIN_len = strlen(BEGIN_DELIMITER);
        rc = strncmp(payload, BEGIN_DELIMITER, BEGIN_len) == 0;
        if (rc)
        {
            const char * base64 = payload + BEGIN_len;
            bool msdos = false;
            if (*base64 == '\r')        // it may be msdos formatted
            {
                msdos = true;
                base64++;
            }
            rc = (*base64 == '\n');
            if (rc)
            {
                base64++;
                const char * p = payload + payload_len - 1;
                rc = (*p == '\0');      // we absolutely insist that PEM be NUL-terminated
                if (rc)
                {
                    p--;
                    rc = (*p == '\n');
                    if (rc)
                    {
                        p--;
                        if (msdos)
                        {
                            rc = (*p == '\r');
                            p--;
                        }
                        if (rc)
                        {
                            size_t END_len = strlen(END_DELIMITER);
                            p -= (END_len - 1);
                            rc = strncmp(p, END_DELIMITER, END_len) == 0;
                            if (rc)
                            {
                                // XXX - jbates - FIX!  not as thorough!!!
                                if (mid != IENVELOPE::RSA_2048_CERT_CHAIN)
                                    rc = Is_PEM_Base64(base64, p - base64);
                            }
                            else {}
                        }
                        else {}
                    }
                    else {}
                }
                else {}
            }
            else {}
        }
        else {}
    }
    else {}

    return rc;
}

// ================================================================================================================================
//
static bool Validate_Entity(uint32_t mid, const char * payload, size_t payload_len)
{
    // we use the range 0x00000000 -> 0x000000FF for VCOM commands.
    // no content inspection at this level for these messages.
    if (mid <= 0xFF)
        return true;
    switch (mid)
    {
        case IENVELOPE::HMAC_SHA_256_128_SECRET_KEY:
        case IENVELOPE::AES_GCM_256_SECRET_KEY:
        case IENVELOPE::AES_GCM_192_SECRET_KEY:
        case IENVELOPE::AES_GCM_128_SECRET_KEY:
        case IENVELOPE::AES_CTR_256_SECRET_KEY:
        case IENVELOPE::AES_CTR_192_SECRET_KEY:
        case IENVELOPE::AES_CTR_128_SECRET_KEY:
        case IENVELOPE::HMAC_SHA_256_FW_AUTH_TOKEN:
            return Binary_Check_Length(mid, payload_len);
        case IENVELOPE::RSA_2048_PUB_KEY:
        case IENVELOPE::RSA_2048_PRIV_KEY:
        case IENVELOPE::RSA_2048_CERT_REQ:
        case IENVELOPE::RSA_2048_CERT_CHAIN:
        case IENVELOPE::RSA_2048_LEAF_CERT:
        case IENVELOPE::RSA_2048_INTER_CA:
        case IENVELOPE::RSA_2048_ROOT_CA:
            return Is_PEM(mid, payload, payload_len);
        case IENVELOPE::FIRMWARE_IMAGE:
        case IENVELOPE::XPRT_VCOM_ENCAPSULATED:
            return true;
    }
    return false;
}

// ================================================================================================================================
//
// Seal()
//
//
// This method encrypts and creates the MAC code for on-wire messages or symmetrically-secured stored objects.
//
//      EKEY        input   128-bit AES-CTR encryption key
//      MKEY        input   128-bit HMAC-SHA-256/128 MAC key
//      MID         input   PKISCMsg Message ID
//      iv          I/O     128-bit IV to use, or NULL, in which case a random IV value will be generated
//                          if provided, the updated IV value will be provided upon successful return.
//                          if provided, and any error, the IV will be randomized, breaking the ability to maintain
//                          a connection-based stream.
//      input       input   data to encrypt and MAC
//      input_len   input   length of data to encrypt and MAC
//      output      input   pointer to output buffer
//      output_len  I/O     in: length of output buffer, out: number of bytes placed in output buffer
//      SEQ         input   optional sequence number, context dependent upon application
//      SLOT        input   optional slot designation, context dependent upon application
//      LAYERS      input   optional layers designation, context dependent upon application
//
// RETURNS:
//
//      0       OK
//     -1       FAIL
//
// ================================================================================================================================

bool oEnvelope::Seal(const unsigned char * EKEY, size_t EKEY_len,
                     const unsigned char * MKEY, size_t MKEY_len,
                     uint32_t mid, unsigned char * iv,
                     const unsigned char * input,  size_t   input_len,
                           unsigned char * output, size_t * output_len,
                     uint32_t      SEQ,
                     unsigned char SLOT,
                     unsigned char LAYERS,
                     unsigned char CIPHERSUITE)

{
    if (!output_len)
        return false;
    if (!(*output_len))
        return false;

    if (CIPHERSUITE == IENVELOPE::CIPHERSUITE_FIPS_AES_GCM)
    {
        // there is no separate MAC key for this envelope type.
        // if not NULL'd here, called C code will reject.
        MKEY = NULL; MKEY_len = 0;
    }

    bool rc = false;

    size_t output_size = *output_len;

    // enforce proper MID / PEM header/footer
    rc = Validate_Entity(mid, reinterpret_cast<const char *>(input), input_len);
    if (rc)
    {
        if (iv)
        {
            switch (CIPHERSUITE)
            {
                case IENVELOPE::CIPHERSUITE_FIPS_AES_HMAC:
                    rc = EnvelopeSeal_aes_hmac(EKEY, EKEY_len, MKEY, MKEY_len, mid, iv,
                                               input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                    break;
                case IENVELOPE::CIPHERSUITE_FIPS_AES_GCM:
                    rc = EnvelopeSeal_aes_gcm(EKEY, EKEY_len, MKEY, MKEY_len, mid, iv,
                                              input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                    break;
                default:
                    rc = false;
                    break;
            }
            if (!rc)
            {
                // destroy iv so that any connection based upon this value will never authenticate
                lumi_random(NULL, iv, ENVELOPE_IV_BYTES);
            }
        }
        else
        {
            AutoHeapBuffer auto_iv(ENVELOPE_IV_BYTES);
            // no iv provided - provide random counter
            lumi_random(NULL, auto_iv.u8Ptr(), auto_iv.Len());
            switch (CIPHERSUITE)
            {
                case IENVELOPE::CIPHERSUITE_FIPS_AES_HMAC:
                    rc = EnvelopeSeal_aes_hmac(EKEY, EKEY_len, MKEY, MKEY_len, mid, auto_iv.u8Ptr(),
                                               input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                    break;
                case IENVELOPE::CIPHERSUITE_FIPS_AES_GCM:
                    rc = EnvelopeSeal_aes_gcm(EKEY, EKEY_len, MKEY, MKEY_len, mid, auto_iv.u8Ptr(),
                                              input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                    break;
                default:
                    rc = false;
                    break;
            }
            // iv is thrown away...
        }
    }

    // if we were not successful, be forceful about it...
    if (!rc)
    {
        memset(output, 0, output_size);
        *output_len = 0;
    }

    return rc;
}

// ================================================================================================================================
//
// Unseal()
//
//
// This method validates the MAC code, and unencrypts on-wire messages or symmetrically-secured stored objects.
// As this method reads what could be wild data, it is very, very particular about its input.
//
//      EKEY        input   128-bit AES-CTR encryption key
//      MKEY        input   128-bit HMAC-SHA-256/128 MAC key
//      iv          I/O     128-bit IV to use, or NULL, in which case a random IV value will be generated
//                          if provided, the updated IV value will be provided upon successful return.
//                          if provided, and any error, the IV will be randomized, breaking the ability to maintain
//                          a connection-based stream.
//      input       input   data to validate MAC and decrypt
//      input_len   input   length of data to validate MAC and decrypt
//      output      input   pointer to output buffer
//      output_len  I/O     in: length of output buffer, out: number of bytes placed in output buffer
//      SEQ         output  optional sequence number found in authenticated header, context dependent upon application
//      SLOT        output  optional slot designation found in authenticated header, context dependent upon application
//      LAYERS      output  optional layers designation found in authenticated header, context dependent upon application
//
// RETURNS:
//
//      0       OK
//     -1       FAIL
//
// ================================================================================================================================

bool oEnvelope::Unseal(const unsigned char * EKEY, size_t EKEY_len,
                       const unsigned char * MKEY, size_t MKEY_len,
                       uint32_t mid, unsigned char * iv,
                       const unsigned char * input,  size_t   input_len,
                             unsigned char * output, size_t * output_len,
                       uint32_t      * SEQ,
                       unsigned char * SLOT,
                       unsigned char * LAYERS,
                       unsigned char * CIPHERSUITE)
{
    if (!output_len)
        return false;
    if (!*output_len)
        return false;

    const USBCB * pUSBCB = reinterpret_cast<const USBCB *>(input);
    unsigned char version_in = ((pUSBCB->ulCommand & ((uint32_t) ENVELOPE_CIPHERSUITE_MASK)) >> ENVELOPE_CIPHERSUITE_SHIFT);

    if (version_in == IENVELOPE::CIPHERSUITE_FIPS_AES_GCM)
    {
        // there is no separate MAC key for this envelope type.
        // if not NULL'd here, called C code will reject.
        MKEY = NULL; MKEY_len = 0;
    }

    bool rc = false;

    uint32_t mid_found;
    size_t output_size = *output_len;
    uint8_t slot_dummy;
    if (!SLOT)
        SLOT = &slot_dummy;
    uint8_t layers_dummy;
    if (!LAYERS)
        LAYERS = &layers_dummy;

    if (iv)
    {
        switch (version_in)
        {
            case IENVELOPE::CIPHERSUITE_FIPS_AES_HMAC:
                rc = EnvelopeUnseal_aes_hmac(EKEY, EKEY_len, MKEY, MKEY_len, &mid_found, iv,
                                             input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                break;
            case IENVELOPE::CIPHERSUITE_FIPS_AES_GCM:
                rc = EnvelopeUnseal_aes_gcm(EKEY, EKEY_len, MKEY, MKEY_len, &mid_found, iv,
                                            input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                break;
            default:
                rc = false;
                break;
        }

        if (!rc)
        {
            // destroy iv so that any connection based upon this value will never authenticate
            lumi_random(NULL, iv, ENVELOPE_IV_BYTES);
        }
    }
    else
    {
        AutoHeapBuffer auto_iv(ENVELOPE_IV_BYTES);
        // XXX - jbates - this presupposes knowledge of the format that envelope.c provides...
        const unsigned char * input_iv = input + sizeof(USBCB);
        memcpy(auto_iv.u8Ptr(), input_iv, auto_iv.Len());
        switch (version_in)
        {
            case IENVELOPE::CIPHERSUITE_FIPS_AES_HMAC:
                rc = EnvelopeUnseal_aes_hmac(EKEY, EKEY_len, MKEY, MKEY_len, &mid_found, auto_iv.u8Ptr(),
                                             input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                break;
            case IENVELOPE::CIPHERSUITE_FIPS_AES_GCM:
                rc = EnvelopeUnseal_aes_gcm(EKEY, EKEY_len, MKEY, MKEY_len, &mid_found, auto_iv.u8Ptr(),
                                            input, input_len, output, output_len, SEQ, SLOT, LAYERS) == 0;
                break;
            default:
                rc = false;
                break;
        }
    }

    if (rc)
    {
        // enforce matching MID
        rc = (mid_found == mid);
        if (rc)
        {
            // enforce proper MID / PEM header/footer
            rc = Validate_Entity(mid, reinterpret_cast<const char *>(output), *output_len);
            if (rc)
            {
                if (CIPHERSUITE)
                    *CIPHERSUITE = version_in;
            }
        }
    }

    // if we were not successful, be forceful about it...
    if (!rc)
    {
        memset(output, 0, output_size);
        *output_len = 0;
    }

    return rc;
}
//
// ================================================================================================================================
