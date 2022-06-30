#pragma once

#define ENVELOPE_IV_BYTES               (16)

// at this layer of the onion, which key slot is this data in regards to?
#define ENVELOPE_SLOT_MASK              (0xFF000000)
#define ENVELOPE_SLOT_SHIFT             (24)
#define ENVELOPE_SLOT_MAX               (0xFF)

// what is the envelope ciphersuite type?
#define ENVELOPE_CIPHERSUITE_MASK       (0x00FF0000)
#define ENVELOPE_CIPHERSUITE_SHIFT      (16)
#define ENVELOPE_CIPHERSUITE_MAX        (0xFF)
// symmetric enveloped packet types
#define ENVELOPE_CIPHERSUITE_FIPS_AES_HMAC (1)//MUST MATCH IENVELOPE_CIPHERSUITE_FIPS_AES_HMAC
#define ENVELOPE_CIPHERSUITE_FIPS_AES_GCM  (2)//MUST MATCH IENVELOPE_CIPHERSUITE_FIPS_AES_GCM

// what is the envelope ciphersuite keying information?
// this is dependent upon the envelope ciphersuite type
// values are defined in envelope_aes_gcm.h and envelope_aes_hmac.h
#define ENVELOPE_KEYING_MASK            (0x0000FF00)
#define ENVELOPE_KEYING_SHIFT           (8)
#define ENVELOPE_KEYING_MAX             (0xFF)
// content of keying information is application-dependent

// reserved bits
// this should checked to be zero to avoid side-channel attacks
#define ENVELOPE_RESERVED_MASK          (0x000000FC)

// how many layers of onion encryption/authentication need to be peeled?
#define ENVELOPE_LAYERS_MASK            (0x00000003)
#define ENVELOPE_LAYERS_SHIFT           (0)
#define ENVELOPE_LAYERS_MAX             (0x03)
