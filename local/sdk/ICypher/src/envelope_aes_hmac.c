/*******************************************************************************
**
**  envelope - ANSI-C version of symmetric datagram envelope methods
**
**  UNIT TEST HINT!
**
**  undefine PADLOCK macros in mbedtls config.h, then...
**
**  gcc -g -DUNIT_TEST_MAIN -DLUMI_USE_POLARSSL_AES_ECB -o envelope -I../../mbedtls-1.3/include -I../include -I../../common/include ../src/envelope.c ../../mbedtls-1.3/library/aes.c ../../mbedtls-1.3/library/sha256.c
**
**  COPYRIGHT INFORMATION:  
**      This software is proprietary and confidential.  
**      By using this software you agree to the terms and conditions of the 
**      associated Lumidigm Inc. License Agreement.
**
**      (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include <stdio.h>
#include "envelope.h"
#include "envelope_aes_hmac.h"
#include <string.h>
#include "polarssl/aes.h"
#include "polarssl/sha256.h"
#include "Platform.h"

#include <application/types_c.h>


#define DEBUG_FAIL
#define DEBUG_RC_FAIL

#define AES_BLOCK_BYTES (16)
#define SHA256_OUT_BYTES (32)
#define ENVELOPE_OVERHEAD(MKEY_len) (sizeof(USBCB) + ENVELOPE_IV_BYTES + sizeof(uint32_t)/*mid*/ + MKEY_len)
#define ENVELOPE_CTR_OFFSET (sizeof(USBCB))
#define ENVELOPE_CTEXT_OFFSET (sizeof(USBCB) + ENVELOPE_IV_BYTES)
// identifying the algorithms used to create the envelope based upon USBCB->ulCommand
#define AES_KEYING_MASK  (0x0000F000)
#define AES_128_CTR      (0x00001000)
#define AES_192_CTR      (0x00002000)
#define AES_256_CTR      (0x00003000)
#define HMAC_KEYING_MASK (0x00000F00)
#define HMAC_SHA_256_128 (0x00000100)
#define HMAC_SHA_256_192 (0x00000200)
#define HMAC_SHA_256     (0x00000300)

// ================================================================================================================================
//
// EnvelopeSeal()
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

int EnvelopeSeal_aes_hmac(const unsigned char * EKEY, size_t EKEY_len,
                          const unsigned char * MKEY, size_t MKEY_len,
                          uint32_t mid, unsigned char * counter,
                          const unsigned char * input,  size_t   input_len,
                                unsigned char * output, size_t * output_len,
                          uint32_t SEQ, unsigned char SLOT, unsigned char LAYERS)
{
    int rc = -1;

    size_t max_out = *output_len;
    *output_len = 0;

    // preconditions...
    if (!EKEY)
    { DEBUG_FAIL; return -1; }
    if (EKEY_len != 16 && EKEY_len != 24 && EKEY_len != 32)
    { DEBUG_FAIL; return -1; }
    if (!MKEY)
    { DEBUG_FAIL; return -1; }
    if (MKEY_len != 16 && MKEY_len != 24 && MKEY_len != 32)
    { DEBUG_FAIL; return -1; }
    if (!counter)
    { DEBUG_FAIL; return -1; }
    if (!input)
    { DEBUG_FAIL; return -1; }
    if (!output)
    { DEBUG_FAIL; return -1; }
    if (!output_len)
    { DEBUG_FAIL; return -1; }
    if (SLOT > ENVELOPE_SLOT_MAX)
    { DEBUG_FAIL; return -1; }
    if (LAYERS > ENVELOPE_LAYERS_MAX)
    { DEBUG_FAIL; return -1; }
    // assure output buffer is of sufficient size...
    // but remember that caller might mistakenly use *output_len as a flag, so we zero it until we are successful...
    if (max_out < input_len + ENVELOPE_OVERHEAD(MKEY_len))
    { DEBUG_FAIL; return -1; }

    // get a block of dynamic memory, and chop it up into the various uses we have for it.
    const size_t alloc_block_size = AES_BLOCK_BYTES + SHA256_OUT_BYTES + sizeof(aes_context) + sizeof(sha256_context);
    unsigned char * alloc_block = (unsigned char *)lumi_malloc(alloc_block_size);
    if (!alloc_block)
    { DEBUG_FAIL; return -1; }
    unsigned char  * buf_stream_block = alloc_block;
    unsigned char  * buf_hmac         = (alloc_block + AES_BLOCK_BYTES);
    aes_context    * aes_ctx          = (aes_context *)(alloc_block + AES_BLOCK_BYTES + SHA256_OUT_BYTES);
    sha256_context * sha256_ctx       = (sha256_context *)(alloc_block + AES_BLOCK_BYTES + SHA256_OUT_BYTES + sizeof(aes_context));

    // clear USBCB struct
    USBCB * pUSBCB = (USBCB *)(output);
    SecureClearBuffer(pUSBCB, sizeof(USBCB));
    pUSBCB->ulCommand = ENVELOPE_CIPHERSUITE_FIPS_AES_HMAC << ENVELOPE_CIPHERSUITE_SHIFT;
    if (EKEY_len == 16)
        pUSBCB->ulCommand |= AES_128_CTR;
    else if (EKEY_len == 24)
        pUSBCB->ulCommand |= AES_192_CTR;
    else if (EKEY_len == 32)
        pUSBCB->ulCommand |= AES_256_CTR;
    else
    { DEBUG_FAIL; return -1; }
    if (MKEY_len == 16)
        pUSBCB->ulCommand |= HMAC_SHA_256_128;
    else if (MKEY_len == 24)
        pUSBCB->ulCommand |= HMAC_SHA_256_192;
    else if (MKEY_len == 32)
        pUSBCB->ulCommand |= HMAC_SHA_256;
    else
    { DEBUG_FAIL; return -1; }
    pUSBCB->ulCommand |= (((uint32_t) SLOT)   << ENVELOPE_SLOT_SHIFT)   & ENVELOPE_SLOT_MASK;
    pUSBCB->ulCommand |= (((uint32_t) LAYERS) << ENVELOPE_LAYERS_SHIFT) & ENVELOPE_LAYERS_MASK;
    pUSBCB->ulData = SEQ;

    // place IV in appropriate place...
    memcpy(output + ENVELOPE_CTR_OFFSET, counter, ENVELOPE_IV_BYTES);

    rc = aes_setkey_enc(aes_ctx, EKEY, (unsigned int)(EKEY_len * 8));
    if (rc == 0)
    {
        size_t nc_off = 0;
        rc = aes_crypt_ctr(aes_ctx, sizeof(mid), &nc_off, counter, buf_stream_block,
                           (unsigned char *) &mid, output + ENVELOPE_CTEXT_OFFSET);
        if (rc == 0)
        {
            rc = aes_crypt_ctr(aes_ctx, input_len, &nc_off, counter, buf_stream_block,
                               input, output + ENVELOPE_CTEXT_OFFSET + sizeof(mid));
            if (rc == 0)
            {
                // at this point: Rudie Can't Fail!
                // prepare for calculating HMAC...
                // USBCB indicates PAYLOAD [ IV + ciphertext + MAC ] length
                pUSBCB->ulCount = (uint32_t)(ENVELOPE_IV_BYTES + sizeof(mid) + input_len + MKEY_len);
                // perform HMAC-SHA-256/128
                // HMAC is performed on [ USBCB + IV + ciphertext ]
                size_t authenticated_bytes = ENVELOPE_CTEXT_OFFSET + sizeof(mid) + input_len;
                // this method returns a int in our embedded version, but void in the PolarSSL OEM version.
                // the only way it can (currently) fail is for a key of length<14B being passed.
                // we specifically exclude this possibility, so we can ignore the return code.
                sha256_hmac_starts(sha256_ctx, MKEY, MKEY_len, 0/*is224*/);
                sha256_hmac_update(sha256_ctx, output, authenticated_bytes);
                sha256_hmac_finish(sha256_ctx, buf_hmac);
                // truncate to MKEY_len bytes
                memcpy(output + authenticated_bytes, buf_hmac, MKEY_len);
                // update output_len so that caller knows how big the transmission / storage unit is.
                *output_len = authenticated_bytes + MKEY_len;
            }
            else
                DEBUG_RC_FAIL;
        }
        else
            DEBUG_RC_FAIL;
    }
    else
        DEBUG_RC_FAIL;

    aes_free(aes_ctx);
    SecureClearBuffer(alloc_block, alloc_block_size);

    return rc;
}

// ================================================================================================================================
//
// EnvelopeUnseal()
//
//
// This method validates the MAC code, and unencrypts on-wire messages or symmetrically-secured stored objects.
// As this method reads what could be wild data, it is very, very particular about its input.
//
//      EKEY        input   128-bit AES-CTR encryption key
//      MKEY        input   128-bit HMAC-SHA-256/128 MAC key
//      counter     I/O     128-bit counter to use, or NULL, in which case a random counter value will be generated
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

int EnvelopeUnseal_aes_hmac(const unsigned char * EKEY, size_t EKEY_len,
                            const unsigned char * MKEY, size_t MKEY_len,
                            uint32_t *mid, unsigned char * counter,
                            const unsigned char * input,  size_t   input_len,
                                  unsigned char * output, size_t * output_len,
                            uint32_t * SEQ, unsigned char * SLOT, unsigned char * LAYERS)
{
    int rc = -1;

    // preconditions...
    if (!EKEY)
    { DEBUG_FAIL; return -1; }
    if (EKEY_len != 16 && EKEY_len != 24 && EKEY_len != 32)
    { DEBUG_FAIL; return -1; }
    if (!MKEY)
    { DEBUG_FAIL; return -1; }
    if (MKEY_len != 16 && MKEY_len != 24 && MKEY_len != 32)
    { DEBUG_FAIL; return -1; }
    if (!counter)
    { DEBUG_FAIL; return -1; }
    if (!input)
    { DEBUG_FAIL; return -1; }
    if (!output)
    { DEBUG_FAIL; return -1; }
    if (!output_len)
    { DEBUG_FAIL; return -1; }
    size_t max_output = *output_len;
    *output_len = 0;
    if (max_output < input_len - ENVELOPE_OVERHEAD(MKEY_len))
    { DEBUG_FAIL; return -1; }
    if (input_len < ENVELOPE_OVERHEAD(MKEY_len) + 1/*min payload*/)
    { DEBUG_FAIL; return -1; }
    // USBCB assertions
    const USBCB * pUSBCB = (const USBCB*) input;
    if (pUSBCB->ulCount != input_len - ENVELOPE_CTR_OFFSET)
    { DEBUG_FAIL; return -1; }
    // version check
    if ((pUSBCB->ulCommand & ((uint32_t) ENVELOPE_CIPHERSUITE_MASK)) >> ENVELOPE_CIPHERSUITE_SHIFT != ENVELOPE_CIPHERSUITE_FIPS_AES_HMAC)
    { DEBUG_FAIL; return -1; }
    // HMAC truncation size check
         if ((pUSBCB->ulCommand & HMAC_KEYING_MASK) == HMAC_SHA_256_128 && MKEY_len == 16)
    {}
    else if ((pUSBCB->ulCommand & HMAC_KEYING_MASK) == HMAC_SHA_256_192 && MKEY_len == 24)
    {}
    else if ((pUSBCB->ulCommand & HMAC_KEYING_MASK) == HMAC_SHA_256     && MKEY_len == 32)
    {}
    else
    { DEBUG_FAIL; return -1; }
    // AES key size check
         if ((pUSBCB->ulCommand & AES_KEYING_MASK) == AES_128_CTR && EKEY_len == 16)
    {}
    else if ((pUSBCB->ulCommand & AES_KEYING_MASK) == AES_192_CTR && EKEY_len == 24)
    {}
    else if ((pUSBCB->ulCommand & AES_KEYING_MASK) == AES_256_CTR && EKEY_len == 32)
    {}
    else
    { DEBUG_FAIL; return -1; }
    if ((pUSBCB->ulCommand & ENVELOPE_RESERVED_MASK) != 0)
    { DEBUG_FAIL; return -1; }
    // counter MUST MATCH!
    const unsigned char * input_counter = input + ENVELOPE_CTR_OFFSET;
    if (SecureCompareBuffer(counter, input_counter, ENVELOPE_IV_BYTES))
    { DEBUG_FAIL; return -1; }

    // get a block of dynamic memory, and chop it up into the various uses we have for it.
    const size_t alloc_block_size = AES_BLOCK_BYTES + SHA256_OUT_BYTES + sizeof(aes_context)+sizeof(sha256_context);
    unsigned char * alloc_block = (unsigned char *)lumi_malloc(alloc_block_size);
    if (!alloc_block)
    {
        DEBUG_FAIL; return -1;
    }
    unsigned char  * buf_stream_block = alloc_block;
    unsigned char  * buf_hmac = (alloc_block + AES_BLOCK_BYTES);
    aes_context    * aes_ctx = (aes_context *)(alloc_block + AES_BLOCK_BYTES + SHA256_OUT_BYTES);
    sha256_context * sha256_ctx = (sha256_context *)(alloc_block + AES_BLOCK_BYTES + SHA256_OUT_BYTES + sizeof(aes_context));

    // HMAC-SHA256/128 outer-layer authentication
    size_t authenticated_bytes = input_len - MKEY_len;
    // this method returns a int in our embedded version, but void in the PolarSSL OEM version.
    // the only way it can (currently) fail is for a key of length<14B being passed.
    // we specifically exclude this possibility, so we can ignore the return code.
    sha256_hmac_starts(sha256_ctx, MKEY, MKEY_len, 0/*is224*/);
    sha256_hmac_update(sha256_ctx, input, authenticated_bytes);
    sha256_hmac_finish(sha256_ctx, buf_hmac);
    // we are only comparing the first 128 bits of the hmac output...
    rc = SecureCompareBuffer(buf_hmac, input + authenticated_bytes, MKEY_len) ? -1 : 0;
    if (rc == 0)
    {
        // AES-128-CTR inner-layer confidentiality
        rc = aes_setkey_enc(aes_ctx, EKEY, (unsigned int)(EKEY_len * 8));
        if (rc == 0)
        {
            // calculate the sub-array to be decrypted
            size_t ciphertext_offset = ENVELOPE_CTEXT_OFFSET;
            size_t ciphertext_len    = input_len - ENVELOPE_OVERHEAD(MKEY_len);
            size_t nc_off = 0;
            uint32_t u32MID = 0xFFFFFFFF;

            // CTR decrypt is the same operation as CTR encrypt
            rc = aes_crypt_ctr(aes_ctx, sizeof(*mid), &nc_off, counter, buf_stream_block,
                               input + ciphertext_offset, (unsigned char *) &u32MID);
            if (rc == 0)
            {
                rc = aes_crypt_ctr(aes_ctx, ciphertext_len, &nc_off, counter, buf_stream_block,
                                   input + ciphertext_offset + sizeof(*mid), output);
                if (rc == 0)
                {
                    *mid = u32MID;
                    *output_len = ciphertext_len;
                    if (SLOT)
                        *SLOT   = (pUSBCB->ulCommand & ((uint32_t) ENVELOPE_SLOT_MASK))   >> ENVELOPE_SLOT_SHIFT;
                    if (LAYERS)
                        *LAYERS = (pUSBCB->ulCommand & ((uint32_t) ENVELOPE_LAYERS_MASK)) >> ENVELOPE_LAYERS_SHIFT;
                    if (SEQ)
                        *SEQ = pUSBCB->ulData;
                    // XXX - jbates - ONLY GOOD FOR NUL-terminated C-string...
                    //DEBUG("%s\n", output);
                }
                else
                    DEBUG_RC_FAIL;
            }
            else
                DEBUG_RC_FAIL;
        }
        else
            DEBUG_RC_FAIL;
    }
    else
        DEBUG_RC_FAIL;

    aes_free(aes_ctx);
    SecureClearBuffer(alloc_block, alloc_block_size);
    lumi_free(alloc_block);

    return rc;
}
