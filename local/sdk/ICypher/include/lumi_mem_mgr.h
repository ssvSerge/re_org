#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define POLARSSL_PLATFORM_MALLOC_MACRO lumi_malloc
#define POLARSSL_PLATFORM_FREE_MACRO   lumi_free 

void * lumi_malloc(size_t);
void lumi_free(void *);

#ifdef __cplusplus
};
#endif
