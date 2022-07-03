// ================================================================================================================================
//
// Platform.h
//
// Various platform helpers
//
// ================================================================================================================================

#include <stddef.h>
#include "lumi_stdint.h"

// ================================================================================================================================
// extern "C", where appropriate...
#ifdef __cplusplus
#define PLATFORM_EXTERN_C "C"
#else
#define PLATFORM_EXTERN_C
#endif

// ================================================================================================================================
// Blackfin memory map defines
#if defined(_VDSP)
#define SECTION_SDRAM0_BANK1 section("sdram0_bank1")
#else
#define SECTION_SDRAM0_BANK1
#endif

// ================================================================================================================================
// platform secure entropy gathering - must be provided by all platforms
extern PLATFORM_EXTERN_C int platform_entropy_init();
extern PLATFORM_EXTERN_C int platform_entropy_callback(void * p_entropy, unsigned char * buf, size_t len);

// ================================================================================================================================
// ZeroMemory
extern PLATFORM_EXTERN_C void SecureClearBuffer(void * p, size_t l);
extern PLATFORM_EXTERN_C int  SecureCompareBuffer(const void * p, const void * q, size_t l);

// ================================================================================================================================
// gmtime_r proxy for X.509 use
#if defined(_WIN32)
#include <windows.h>
#define gmtime_r(tp,tm) gmtime_s(tm,tp)
#endif

// ================================================================================================================================
//
#undef PLATFORM_EXTERN_C

// ================================================================================================================================
