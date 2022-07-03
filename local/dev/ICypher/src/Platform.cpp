// ================================================================================================================================
//
// Platform.cpp
//
// Various platform helpers
//
// ================================================================================================================================

#include "PlatformDev.h"
#include "IMemMgr.h"
#include "lumi_mem_mgr.h"

// ================================================================================================================================
// proxies for polarssl_malloc() / polarssl_free()
void * lumi_malloc(size_t l)
{
    return MALLOC(l);
}

void lumi_free(void * p)
{
    FREE(p);
}

// ================================================================================================================================
// method to clear a buffer that will not be optimized out
void SecureClearBuffer(void * p, size_t l)
{
    volatile unsigned char *v = reinterpret_cast<volatile unsigned char *>(p);
    while (l--)
        *v++ = 0;
}

// ================================================================================================================================
// method to clear a buffer that will not be optimized out
int SecureCompareBuffer(const void * p, const void * q, size_t l)
{
    const uint8_t * p8 = reinterpret_cast<const uint8_t *>(p);
    const uint8_t * q8 = reinterpret_cast<const uint8_t *>(q);
    uint8_t rc = 0;
    while (l--)
    {
        rc |= (*p8) ^ (*q8);
        p8++;
        q8++;
    }
    return rc;
}

// ================================================================================================================================
