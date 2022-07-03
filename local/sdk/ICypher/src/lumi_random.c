#include <stdio.h>
#include <string.h>

#include "polarssl/ctr_drbg.h"
#include "lumi_random.h"
#include "Platform.h"

static const unsigned char * lumi_random_cursor  = NULL;
static int                   lumi_random_remain  = -1;
static void                * lumi_random_context = NULL;
static const unsigned char * lumi_random_static_seed = NULL;
SECTION_SDRAM0_BANK1
ctr_drbg_context   g_ctr_drbg_ctx;        // used externally to this module - i.e. bignum.c prime generation, so not static

#ifdef __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int                g_fd_platform_entropy = -1;

int platform_entropy_callback(void * p_entropy, unsigned char * buf, size_t len)
{
    int platform_entropy_init();
    int rc = -1;

    //DEBUG("platform_entropy_callback: %p %p %zd - ", p_entropy, buf, len);
    if (g_fd_platform_entropy != -1 || platform_entropy_init() == 0)
    {
        int got = read(g_fd_platform_entropy, buf, len);
        if (got == len)
            rc = 0;
    }

    return rc;
}

int platform_entropy_init()
{
    int rc = -1;

    g_fd_platform_entropy = open("/dev/urandom", O_RDONLY);
    rc = (g_fd_platform_entropy == -1);
    if (rc == 0)
    {
        // self-test
        rc = ctr_drbg_self_test(1/*verbose*/);
    }
    //else DEBUG_RC_FAIL;

    return rc;
}
#elif defined(_VDSP)
#include <stdint.h>

/* pRandom must have room for 32 bytes.... */
extern bool GetSERandomNumber(unsigned char* pRandom, int Parm1);
#define SE_NONCE_LENGTH    32

int                g_fd_platform_entropy = -1;

int platform_entropy_callback(void * p_entropy, unsigned char * buf, size_t len)
{
    int platform_entropy_init(void);
    int rc = -1;

    
    //  JACK - has to be 32 bytes...you can do the Math below...
    unsigned char atshaRand[SE_NONCE_LENGTH];

    if (g_fd_platform_entropy != -1 || platform_entropy_init() == 0)
    {
        int iter = len/SE_NONCE_LENGTH;
        int remain = len%SE_NONCE_LENGTH;
        int ii;
        for(ii = 0 ; ii < iter; ii++)
        {
            if(false == GetSERandomNumber(atshaRand, 1))
            {
                return rc;
            }
            memcpy(buf+ii*SE_NONCE_LENGTH, atshaRand, SE_NONCE_LENGTH);
        }
        if(remain)
        {
            if(false == GetSERandomNumber(atshaRand, 1))
            {
                return rc;
            }
            memcpy(buf+ii*SE_NONCE_LENGTH, atshaRand, remain);
        }
        rc =0;

    }
    return rc;

}

int platform_entropy_init()
{
    int rc = 0;
    g_fd_platform_entropy =0;
    rc = ctr_drbg_self_test(0/*verbose*/);

    
    return rc;
}
#else
#include <Windows.h>
HCRYPTPROV                g_fd_platform_entropy = 0;

int platform_entropy_callback(void * p_entropy, unsigned char * buf, size_t len)
{
    int platform_entropy_init();
    int rc = -1;

    if (g_fd_platform_entropy != 0 || platform_entropy_init() == 0)
    {
        if (!CryptGenRandom(g_fd_platform_entropy, (DWORD)len, (BYTE*)buf))
        {
            //
        }
        else
        {
            rc = 0;
        }            
    }

    return rc;
}

int platform_entropy_init()
{
    int rc = -1;
    
    rc = !CryptAcquireContextW((HCRYPTPROV *)(&g_fd_platform_entropy), 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    if (rc == 0)
    {
        // self-test
        rc = ctr_drbg_self_test(0/*verbose*/);
    }
    //else DEBUG_RC_FAIL;

    return rc;
}
#endif

int static_seed_entropy_callback(void * p_entropy, unsigned char * buf, size_t len)
{
    #if defined(NO_STATIC_RANDOM)
        return -1;
    #endif
    if (len != 48)
        return -1;
    if (lumi_random_static_seed == NULL)
        return -1;
    memcpy(buf, lumi_random_static_seed, len);
    //lumi_random_static_seed = NULL;
    //fprintf(stdout, "WARNING: static seed test mode active - %zd seed bytes returned.\n", len);
    return 0;
}

static void drbg_init(void)
{
    // bring up the global CTR DRBG generator
    int rc;
    if (lumi_random_static_seed != NULL)
    {
        fprintf(stdout, "WARNING: static seed provided and random prediction resistance is being disabled\n");
    #if defined(NO_STATIC_RANDOM)
        return;
    #endif
    }
    if (lumi_random_static_seed)
        rc = ctr_drbg_init(&g_ctr_drbg_ctx, static_seed_entropy_callback, NULL, NULL, 0);
    else
        rc = ctr_drbg_init(&g_ctr_drbg_ctx, platform_entropy_callback, NULL, NULL, 0);
    if (rc != 0)
        return;
    ctr_drbg_set_prediction_resistance(&g_ctr_drbg_ctx, (lumi_random_static_seed == NULL));
    lumi_random_context = &g_ctr_drbg_ctx;
    //fprintf(stdout, "INFO: AES_CTR_DRBG initialized\n");
}

void lumi_random_init(const unsigned char * buffer, size_t len)
{
    if (buffer && len)
    {
        //fprintf(stdout, "WARNING: random test mode init'd - %zd bytes buffered.\n", len);
        lumi_random_cursor = buffer;
        lumi_random_remain = (int)len;
    }
    else if (buffer || len)
    {
        fprintf(stdout, "ERROR: random test mode initialization failure!\n");
        lumi_random_cursor = NULL;
        lumi_random_remain = -1;
    }
    else
    {
        //fprintf(stdout, "INFO: random test mode disabled.\n");
        lumi_random_cursor = NULL;
        lumi_random_remain = -1;
    }
    if (lumi_random_context == NULL)
    {
        drbg_init();
    }
}

void lumi_random_static_seed_init(const unsigned char * buffer, size_t len)
{
    if (buffer && len == 48)
        lumi_random_static_seed = buffer;
    else
        lumi_random_static_seed = NULL;

    // clear any context.
    if (lumi_random_context)
        ctr_drbg_free(lumi_random_context);

    // clear our state...
    lumi_random_cursor  = NULL;
    lumi_random_remain  = -1;
    lumi_random_context = NULL;
 
    drbg_init();
}

int lumi_random_test_mode_remain()
{
    #if defined(NO_STATIC_RANDOM)
        return -1;
    #endif
    return lumi_random_remain;
}

// when calling this, send NULL for p_rng.
// it only exists to fulfill the API requirements for a PolarSSL random source.
int lumi_random(void * p_rng, unsigned char * output, size_t output_len)
{
    if (lumi_random_context == NULL)
    {
        drbg_init();
    }
    if (!lumi_random_cursor)
        return ctr_drbg_random(lumi_random_context, output, output_len);
    #if defined(NO_STATIC_RANDOM)
        return ctr_drbg_random(lumi_random_context, output, output_len);
    #endif
    if (lumi_random_remain < (int)output_len)
    {
        fprintf(stdout, "ERROR: random test mode out of entropy - remain=%d < request=%d!\n", lumi_random_remain, (int)output_len);
        return POLARSSL_ERR_CTR_DRBG_REQUEST_TOO_BIG;
    }
    memcpy(output, lumi_random_cursor, output_len);
    lumi_random_remain -= (int)output_len;
    lumi_random_cursor += output_len;
    //fprintf(stdout, "WARNING: random test mode active - %zd bytes returned.\n", output_len);
    // NOTE: we will not automatically clear lumi_random_cursor, as we want failures to occur
    //       if test mode is active, and test entropy is exhausted...
    //       i.e. we want to hit the lumi_random_remain < output_len test, above...
    return 0;
}
