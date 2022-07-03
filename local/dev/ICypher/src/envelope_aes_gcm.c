/*******************************************************************************
**
**  envelope - ANSI-C version of symmetric datagram envelope methods
**             AES GCM version.
**
**             XXX - jbates - THIS CODE USES GLOBAL STATIC VARIABLES.
**                            THIS CODE IS NOT RE-ENTRANT!
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

#include "envelope.h"
#include "envelope_aes_gcm.h"
#include "lumi_random.h"
#include <string.h>
#include "polarssl/gcm.h"
#include "Platform.h"
#include "lumi_mem_mgr.h"

#include <application/types_c.h>

#define DEBUG_FAIL
#define DEBUG_RC_FAIL

#define ENVELOPE_MAC_BYTES 16
#define ENVELOPE_OVERHEAD (sizeof(USBCB) + ENVELOPE_IV_BYTES + sizeof(uint32_t)/*mid*/ + ENVELOPE_MAC_BYTES)
#define ENVELOPE_IV_OFFSET (sizeof(USBCB))
#define ENVELOPE_MID_OFFSET (ENVELOPE_IV_OFFSET + ENVELOPE_IV_BYTES)
#define ENVELOPE_PAYLOAD_OFFSET (ENVELOPE_MID_OFFSET + sizeof(uint32_t/*mid_t*/))
#define ENVELOPE_MAC_OFFSET(payload_len) (ENVELOPE_PAYLOAD_OFFSET + payload_len)
// identifying the algorithms used to create the envelope based upon USBCB->ulCommand
#define AES_128_GCM      (0x00000100)
#define AES_192_GCM      (0x00000200)
#define AES_256_GCM      (0x00000400)

void SecureClearBuffer(void * p, size_t l);
int  SecureCompareBuffer(const void * p, const void * q, size_t l);

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

int EnvelopeSeal_aes_gcm(const unsigned char * EKEY, size_t EKEY_len,
                         const unsigned char * MKEY, size_t MKEY_len,
                         uint32_t mid, unsigned char * iv,
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
    if (MKEY)                   // no mac key for this type of envelope
    { DEBUG_FAIL; return -1; }
    if (MKEY_len != 0)          // no mac key for this type of envelope
    { DEBUG_FAIL; return -1; }
    if (!iv)
    { DEBUG_FAIL; return -1; }
    if (!input)
    { DEBUG_FAIL; return -1; }
    if (!output)
    { DEBUG_FAIL; return -1; }
    if (!output_len)
    { DEBUG_FAIL; return -1; }
    // assure output buffer is of sufficient size...
    // but remember that caller might mistakenly use *output_len as a flag, so we zero it until we are successful...
    if (max_out < input_len + ENVELOPE_OVERHEAD)
    { DEBUG_FAIL; return -1; }
    if (SLOT > ENVELOPE_SLOT_MAX)
    { DEBUG_FAIL; return -1; }
    if (LAYERS > ENVELOPE_LAYERS_MAX)
    { DEBUG_FAIL; return -1; }

    // clear USBCB struct
    USBCB * pUSBCB = (USBCB *)(output);
    SecureClearBuffer(pUSBCB, sizeof(USBCB));
    pUSBCB->ulCommand = ((uint32_t) ENVELOPE_CIPHERSUITE_FIPS_AES_GCM) << ENVELOPE_CIPHERSUITE_SHIFT;
    if (EKEY_len == 16)
    {
        EKEY_len = 128;
        pUSBCB->ulCommand |= AES_128_GCM;
    }
    else if (EKEY_len == 24)
    {
        EKEY_len = 192;
        pUSBCB->ulCommand |= AES_192_GCM;
    }
    else if (EKEY_len == 32)
    {
        pUSBCB->ulCommand |= AES_256_GCM;
        EKEY_len = 256;
    }
    else
    { DEBUG_FAIL; return -1; }
    gcm_context * pGCMCtx = (gcm_context *)lumi_malloc(sizeof(gcm_context));
    if (!pGCMCtx)
    { DEBUG_FAIL; return -1; }

    pUSBCB->ulCommand |= (((uint32_t) SLOT)   << ENVELOPE_SLOT_SHIFT)   & ENVELOPE_SLOT_MASK;
    pUSBCB->ulCommand |= (((uint32_t) LAYERS) << ENVELOPE_LAYERS_SHIFT) & ENVELOPE_LAYERS_MASK;
    pUSBCB->ulData = SEQ;
    // have to do this before calculating MAC
    pUSBCB->ulCount = (uint32_t)(ENVELOPE_IV_BYTES + sizeof(mid) + input_len + ENVELOPE_MAC_BYTES);

    // get IV and copy to appropriate place...
    rc = lumi_random(NULL/*p_rng*/, iv, ENVELOPE_IV_BYTES);
    if (rc == 0)
    {
        memcpy(output + ENVELOPE_IV_OFFSET, iv, ENVELOPE_IV_BYTES);
        rc = gcm_init(pGCMCtx, POLARSSL_CIPHER_ID_AES, EKEY, EKEY_len);
        if (rc == 0)
        {
            rc = gcm_starts(pGCMCtx, GCM_ENCRYPT, iv, ENVELOPE_IV_BYTES, (unsigned char*)pUSBCB, sizeof(USBCB));
            if (rc == 0)
            {
                unsigned char * mid_out = output + ENVELOPE_MID_OFFSET;
                rc = gcm_update(pGCMCtx, sizeof(mid), (unsigned char *)&mid, mid_out);
                if (rc == 0)
                {
                    unsigned char * payload_out = mid_out + sizeof(mid);
                    rc = gcm_update(pGCMCtx, input_len, input, payload_out);
                    if (rc == 0)
                    {
                        unsigned char * mac_out = payload_out + input_len;
                        rc = gcm_finish(pGCMCtx, mac_out, ENVELOPE_MAC_BYTES);
                        if (rc == 0)
                        {
                            *output_len = input_len + ENVELOPE_OVERHEAD;
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
        }
        else
            DEBUG_RC_FAIL;
    }
    else
        DEBUG_RC_FAIL;

    gcm_free(pGCMCtx);
    lumi_free(pGCMCtx);

    // because this is pre-calculated before the MAC, if anything failed, clear.
    if (rc != 0)
        pUSBCB->ulCount = 0;

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

int EnvelopeUnseal_aes_gcm(const unsigned char * EKEY, size_t EKEY_len,
                           const unsigned char * MKEY, size_t MKEY_len,
                           uint32_t *pmid, unsigned char * iv,
                           const unsigned char * input,  size_t   input_len,
                                 unsigned char * output, size_t * output_len,
                           uint32_t * SEQ, unsigned char * SLOT, unsigned char * LAYERS)
{
    int rc = -1;

    unsigned char tag[ENVELOPE_MAC_BYTES];

    // preconditions...
    if (!EKEY)
    { DEBUG_FAIL; return -1; }
    if (EKEY_len != 16 && EKEY_len != 24 && EKEY_len != 32)
    { DEBUG_FAIL; return -1; }
    if (MKEY)                   // no mac key for this type of envelope
    { DEBUG_FAIL; return -1; }
    if (MKEY_len != 0)          // no mac key for this type of envelope
    { DEBUG_FAIL; return -1; }
    if (!iv)
    { DEBUG_FAIL; return -1; }
    if (!input)
    { DEBUG_FAIL; return -1; }
    if (!output)
    { DEBUG_FAIL; return -1; }
    if (!output_len)
    { DEBUG_FAIL; return -1; }
    size_t max_output = *output_len;
    *output_len = 0;
    if (max_output < input_len - ENVELOPE_OVERHEAD)
    { DEBUG_FAIL; return -1; }
    if (input_len < ENVELOPE_OVERHEAD + 1/*min payload*/)
    { DEBUG_FAIL; return -1; }
    // USBCB assertions
    const USBCB * pUSBCB = (const USBCB*) input;
    if (pUSBCB->ulCount != input_len - ENVELOPE_IV_OFFSET)
    { DEBUG_FAIL; return -1; }
    // version check
    if ((pUSBCB->ulCommand & ((uint32_t) ENVELOPE_CIPHERSUITE_MASK)) >> ENVELOPE_CIPHERSUITE_SHIFT != ENVELOPE_CIPHERSUITE_FIPS_AES_GCM)
    { DEBUG_FAIL; return -1; }
    // AES key size check
         if ((pUSBCB->ulCommand & ENVELOPE_KEYING_MASK) == AES_128_GCM && EKEY_len == 16)
    {
        EKEY_len = 128;
    }
    else if ((pUSBCB->ulCommand & ENVELOPE_KEYING_MASK) == AES_192_GCM && EKEY_len == 24)
    {
        EKEY_len = 192;
    }
    else if ((pUSBCB->ulCommand & ENVELOPE_KEYING_MASK) == AES_256_GCM && EKEY_len == 32)
    {
        EKEY_len = 256;
    }
    else
    { DEBUG_FAIL; return -1; }
    if ((pUSBCB->ulCommand & ENVELOPE_RESERVED_MASK) != 0)
    { DEBUG_FAIL; return -1; }
    // iv MUST MATCH!
    const unsigned char * input_iv = input + ENVELOPE_IV_OFFSET;
    if (SecureCompareBuffer(iv, input_iv, ENVELOPE_IV_BYTES))
    { DEBUG_FAIL; return -1; }

    gcm_context * pGCMCtx = (gcm_context *)lumi_malloc(sizeof(gcm_context));
    if (!pGCMCtx)
    { DEBUG_FAIL; return -1; }
    rc = gcm_init(pGCMCtx, POLARSSL_CIPHER_ID_AES, EKEY, EKEY_len);
    if (rc == 0)
    {
        rc = gcm_starts(pGCMCtx, GCM_DECRYPT, iv, ENVELOPE_IV_BYTES, (unsigned char* ) pUSBCB, sizeof(USBCB));
        if (rc == 0)
        {
            const unsigned char * mid_in = input + ENVELOPE_MID_OFFSET;
            rc = gcm_update(pGCMCtx, sizeof(*pmid), mid_in, (unsigned char *) pmid);
            if (rc == 0)
            {
                size_t payload_len = input_len - ENVELOPE_OVERHEAD;
                const unsigned char * payload_in = mid_in + sizeof(*pmid);
                rc = gcm_update(pGCMCtx, payload_len, payload_in, output);
                if (rc == 0)
                {
                    rc = gcm_finish(pGCMCtx, tag, ENVELOPE_MAC_BYTES);
                    if (rc == 0)
                    {
                        const unsigned char * mac_in = payload_in + payload_len;
                        rc = SecureCompareBuffer(mac_in, tag, ENVELOPE_MAC_BYTES);
                        if (rc == 0)
                        {
                            *output_len = payload_len;
                            if (SLOT)
                                *SLOT   = (pUSBCB->ulCommand & ((uint32_t) ENVELOPE_SLOT_MASK))   >> ENVELOPE_SLOT_SHIFT;
                            if (LAYERS)
                                *LAYERS = (pUSBCB->ulCommand & ((uint32_t) ENVELOPE_LAYERS_MASK)) >> ENVELOPE_LAYERS_SHIFT;
                            if (SEQ)
                                *SEQ = pUSBCB->ulData;
                        }
                        else DEBUG_RC_FAIL;
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
    }
    else
        DEBUG_RC_FAIL;

    gcm_free(pGCMCtx);
    lumi_free(pGCMCtx);

    return rc;
}
