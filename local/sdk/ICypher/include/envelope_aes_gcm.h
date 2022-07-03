/*******************************************************************************
**
**    envelope - ANSI-C version of symmetric datagram envelope methods
**
**             XXX - jbates - THIS CODE USES GLOBAL STATIC VARIABLES.
**                            THIS CODE IS NOT RE-ENTRANT!
**
**    COPYRIGHT INFORMATION:    
**        This software is proprietary and confidential.  
**        By using this software you agree to the terms and conditions of the 
**        associated Lumidigm Inc. License Agreement.
**
**        (c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include <stddef.h>
#include "lumi_stdint.h"

#ifdef __cplusplus
#define ENVELOPE_EXTERN_C "C"
#else
#define ENVELOPE_EXTERN_C
#endif

// ================================================================================================================================
//
#if 0
// here is a structure and annotation that explains the format of sealed envelopes
typedef struct
{
    struct USBCB usbcb;     // plaintext, authenticated:  USBCB struct for interpretation
    unsigned char IV[16];   // plaintext, authenticated:  AES IV
    uint32_t mid            // ciphertext, authenticated: application-level discriminator for interpreting payload
    unsigned char text[];   // ciphertext, authenticated: payload
    unsigned char MAC[16];  // plaintext, authenticator.
} EnvelopeSealedFormat;
#endif

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

extern ENVELOPE_EXTERN_C int EnvelopeSeal_aes_gcm(const unsigned char * EKEY, size_t EKEY_len,
                                                  const unsigned char * MKEY, size_t MKEY_len,
                                                  uint32_t mid, unsigned char * iv,
                                                  const unsigned char * input,  size_t   input_len,
                                                        unsigned char * output, size_t * output_len,
                                                  uint32_t SEQ, unsigned char SLOT, unsigned char LAYERS);

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

extern ENVELOPE_EXTERN_C int EnvelopeUnseal_aes_gcm(const unsigned char * EKEY, size_t EKEY_len,
                                                    const unsigned char * MKEY, size_t MKEY_len,
                                                    uint32_t *mid, unsigned char * iv,
                                                    const unsigned char * input,  size_t   input_len,
                                                          unsigned char * output, size_t * output_len,
                                                    uint32_t * SEQ, unsigned char * SLOT, unsigned char * LAYERS);
//
// ================================================================================================================================
#undef ENVELOPE_EXTERN_C
