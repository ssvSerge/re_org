#pragma once
#if defined(_VDSP)
#include <string.h>

#ifdef __cplusplus
  extern "C" {
#endif
int     strcasecmp(const char *_s1, const char *_s2);
int     strncasecmp(const char *_s1, const char *_s2, size_t _n);

#ifdef __cplusplus
  }
#endif
#endif

