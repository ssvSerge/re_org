#ifndef __Endian_h__
#define __Endian_h__

#ifdef WIN32
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif
#include <stdio.h>

typedef union
{
    uint32_t i;
    char c[sizeof(uint32_t)];
} t_ENDIAN_OBSERVER;
static t_ENDIAN_OBSERVER s_ENDIAN_OBSERVER = {0x03020100};

#define IsBigEndian()    (s_ENDIAN_OBSERVER.c[0] == 0x03)
#define IsLittleEndian() (s_ENDIAN_OBSERVER.c[0] == 0x00)

#define endian_swap16(s)    (                               \
                                (((s) & 0x00FF) << 8)     | \
                                (((s) & 0xFF00) >> 8)       \
                            )

#define endian_swap32(l)    (                                       \
                                (((l) & 0x000000FFL) << 24)       | \
                                (((l) & 0x0000FF00L) <<  8)       | \
                                (((l) & 0x00FF0000L) >>  8)       | \
                                (((l) & 0xFF000000L) >> 24)         \
                            )

#define endian_swap64(ll)    (                                               \
                                (((ll) & 0x00000000000000FFLL) << 56)     | \
                                (((ll) & 0x000000000000FF00LL) << 40)     | \
                                (((ll) & 0x0000000000FF0000LL) << 24)     | \
                                (((ll) & 0x00000000FF000000LL) <<  8)     | \
                                (((ll) & 0x000000FF00000000LL) >>  8)     | \
                                (((ll) & 0x0000FF0000000000LL) >> 24)     | \
                                (((ll) & 0x00FF000000000000LL) >> 40)     | \
                                (((ll) & 0xFF00000000000000LL) >> 56)       \
                            )

#define Ntoh16(s)  ( IsLittleEndian() ? endian_swap16(s)  : (s)  )
#define Hton16(s)  ( IsLittleEndian() ? endian_swap16(s)  : (s)  )
#define Ntoh32(l)  ( IsLittleEndian() ? endian_swap32(l)  : (l)  )
#define Hton32(l)  ( IsLittleEndian() ? endian_swap32(l)  : (l)  )
#define Ntoh64(ll) ( IsLittleEndian() ? endian_swap64(ll) : (ll) )
#define Hton64(ll) ( IsLittleEndian() ? endian_swap64(ll) : (ll) )

#endif
