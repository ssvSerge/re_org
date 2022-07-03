#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int mode_t;

int write(int fd, const void *buf, int count);
int read(int fd, void* buf, size_t count);
int close(int fd);
int open(const char *pathname, int flags);


#ifdef __cplusplus
}
#endif
