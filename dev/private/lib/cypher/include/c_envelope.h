#pragma once

/*******************************************************************************
**
**	c_envelope - symmetric datagram envelope methods
**
**	COPYRIGHT INFORMATION:	
**		This software is proprietary and confidential.  
**		By using this software you agree to the terms and conditions of the 
**		associated Lumidigm Inc. License Agreement.
**
**		(c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/

#include "ICypher.h"
#include "usbcmdset.h"

class oEnvelope : public IENVELOPE
{
public:
                  oEnvelope() {}
    virtual       ~oEnvelope() {}

    // ============================================================================================================================
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
	//      CIPHERSUITE input   specfic envelope type to use from IENVELOPE::CIPHERSUITE
    //
    // RETURNS:
    //
    //      true    OK
    //      false   FAIL
    //
    // ============================================================================================================================
    virtual bool   Seal(const unsigned char * EKEY, size_t EKEY_len,
                        const unsigned char * MKEY, size_t MKEY_len,
                        uint32_t mid, unsigned char * iv,
                        const unsigned char * input,  size_t   input_len,
                              unsigned char * output, size_t * output_len,
						uint32_t      SEQ         = 0,
                        unsigned char SLOT        = 0,
                        unsigned char LAYERS      = 0,
                        unsigned char CIPHERSUITE = IENVELOPE::CIPHERSUITE_FIPS_AES_HMAC);

    // ============================================================================================================================
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
	//      CIPHERSUITE output  specfic envelope type found in authenticated header, from IENVELOPE::CIPHERSUITE
    //
    // RETURNS:
    //
    //      true    OK
    //      false   FAIL
    //
    // ============================================================================================================================
    virtual bool Unseal(const unsigned char * EKEY, size_t EKEY_len,
                        const unsigned char * MKEY, size_t MKEY_len,
                        uint32_t mid, unsigned char * iv,
                        const unsigned char * input,  size_t   input_len,
                              unsigned char * output, size_t * output_len,
		                uint32_t      * SEQ         = NULL,
                        unsigned char * SLOT        = NULL,
                        unsigned char * LAYERS      = NULL,
                        unsigned char * CIPHERSUITE = NULL);
};
