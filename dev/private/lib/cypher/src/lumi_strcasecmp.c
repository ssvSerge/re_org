#if defined(_VDSP)
#define __NO_BUILTIN
#include "lumi_strcasecmp.h"
#include "ctype.h"

/* Compare null-terminated strings */
int strcasecmp(const char *s1, const char *s2)
{
    for (; tolower(*s1) == tolower(*s2); ++s1, ++s2) {
        /* while equal */
        if ('\0' == *s2)
            return 0;            /* end of equal strings */
    }

    /* C std requires unsigned char comparison */
    return ((tolower(*(unsigned char*)s1) > tolower(*(unsigned char*)s2) )? 1 : -1);
}

int strncasecmp(const char *s1, const char *s2, size_t cnt)
{
    for (; cnt > 0; --cnt, ++s1, ++s2) {
        if (tolower(*s1) != tolower(*s2)) { /* first mismatch */
            /* the standard requires unsigned char comparison */
            return (tolower(*(unsigned char*)s1) > tolower(*(unsigned char*)s2) ? 1 : -1);
        }
        else if ('\0' == *s2) { /* chars are equal and null */
            return 0;
        }
    }
    return 0; /* compared cnt characters, found no mismatches */
}

#endif
