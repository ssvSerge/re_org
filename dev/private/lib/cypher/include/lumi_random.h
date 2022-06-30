// ================================================================================================================================
//
#pragma once

// ================================================================================================================================
//
#ifdef __cplusplus
extern "C" {
#endif

// ================================================================================================================================
// initialize a test mode static entropy sequence by calling this
// this may be called at any time in a program's execution
// callers to lumi_random() will receive EXACTLY the bytes of the passed buffer
// multiple calls to lumi_random() are allowed to consume the entire buffer
extern void lumi_random_init(const unsigned char * buffer, size_t len);

// ================================================================================================================================
// initialize a test mode static-seeded entropy sequence by calling this
// this may be called at any time in a program's execution
// callers to lumi_random() will receive output from AES_CTR_DRBG with this seed
// AES_CTR_DRBG prediction resistance will be turned off
// len MUST == 48
// multiple calls to lumi_random() are allowed and AES_CTR_DRBG will not use platform entropy during reseeding
// so note this: every time ctr_drbg_reseed() is called the same 48 bytes are returned.
//               this is hokey, but yields a deterministic result without having to have huge entropy files
//               associated with the NIST tests.
extern void lumi_random_static_seed_init(const unsigned char * buffer, size_t len);

// ================================================================================================================================
// determine if test mode entropy is active by calling this.
// returns: -1 test mode not active
//           0 test mode entropy exhausted
//          >1 test mode entropy remaining
extern int lumi_random_test_mode_remain();

// ================================================================================================================================
// get entropy.  pass p_rng = NULL, as this paramter exists solely to satisfy PolarSSL entropy source function prototypes.
// if test mode is active, then entropy is emulated from the buffer passed during lumi_random_init.
// if test mode is not active, AES CTR DRBG is used, seeded from platform entropy.
extern int  lumi_random(void * p_rng, unsigned char * output, size_t output_len);

#ifdef __cplusplus
};
#endif

//
// ================================================================================================================================
