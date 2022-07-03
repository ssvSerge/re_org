#include <string.h>
#include "lumi_stdint.h"
#include "envelope_aes_gcm.h"

#include <application/types_c.h>


static int envelope_aes_gcm_self_test_inner(size_t EKEY_len)
{
    int i;
    uint8_t input[64];
    uint8_t counter[16];
    for (i = 0; i < 16; i++)
    { counter[i] = i; }
    uint8_t sealed[128];
    size_t sealed_len = sizeof(sealed);
    uint8_t EKEY[32];
    for (i = 0; i < 32; i++)
    { EKEY[i] = i; }
    // uint8_t MKEY[32];    -- no MKEY for GCM mode.
    uint32_t mid_in = 0x01020304;

    uint32_t mid_out;
    unsigned char * input_counter = sealed + sizeof(USBCB);
    uint8_t input_again[sizeof(input)];
    size_t input_size = sizeof(input_again);
    unsigned char SLOT = 0;
    unsigned char LAYERS = 0;
    uint32_t SEQ = 0;

    //memset(input, 0x80, sizeof(input));
    memcpy(input, " 123456789 123456789 123456789 123456789 123456789 123456789 123", sizeof(input));
    memset(sealed, 0xFF, sizeof(sealed));
    memset(input_again, 0xFF, sizeof(input_again));

    if (EnvelopeSeal_aes_gcm(EKEY, EKEY_len, NULL, 0,
                             mid_in, counter,
                             input, sizeof(input),
                             sealed, &sealed_len,
                             SEQ, SLOT, LAYERS))
    {
        return -1;
    }
    if (EnvelopeUnseal_aes_gcm(EKEY, EKEY_len, NULL, 0,
                               &mid_out, input_counter,
                               sealed, sealed_len,
                               input_again, &input_size,
                               &SEQ, &SLOT, &LAYERS))
    {
        return -1;
    }
    if (input_size != sizeof(input))
    {
        return -1;
    }
    if (mid_in != mid_out)
    {
        return -1;
    }
    if (SLOT != 0)
    {
        return -1;
    }
    if (LAYERS != 0)
    {
        return -1;
    }
    if (memcmp(input, input_again, input_size))
    {
        return -1;
    }
    return 0;
}

int envelope_aes_gcm_self_test()
{
    if (envelope_aes_gcm_self_test_inner(16) != 0)
        return -1;
    if (envelope_aes_gcm_self_test_inner(24) != 0)
        return -1;
    if (envelope_aes_gcm_self_test_inner(32) != 0)
        return -1;
    return 0;
}
