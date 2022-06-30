#ifndef __cypher_platform_h__
#define __cypher_platform_h__

#ifndef SECTION_SDRAM0_BANK0
#define SECTION_SDRAM0_BANK0
#endif

#ifndef SECTION_SDRAM0_BANK1
#define SECTION_SDRAM0_BANK1
#endif


#include <stdint.h>

void SecureClearBuffer   ( void* ptr, int len );
int  SecureCompareBuffer ( const void* p1, const void* p2, int len );

#endif
