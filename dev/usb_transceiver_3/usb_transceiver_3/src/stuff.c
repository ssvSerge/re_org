#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/select.h>

#include <unistd.h>
#include <libaio.h>

#ifdef __cplusplus 
extern "C" {
#endif

uint32_t le32toh(uint32_t little_endian_32bits) {

    return little_endian_32bits;
}

uint16_t le16toh(uint16_t little_endian_16bits) {

    return little_endian_16bits;
}

sighandler_t signal(int signum, sighandler_t handler) {

    return 0;
}

void _hb_msg(unsigned level, const char* fmt, ...) {

}

void set_log_file(FILE * fFile) {

}

void set_verbosity(int32_t verbosity) {

}

int is_log_file_set() {

    return 0;
}

void close_log_file() {

}

int read(int fd, void* buf, size_t count) {

    return 0;
}

int write(int fd, const void* buf, int count) {

    return 0;
}

int close(int fd) {

    return 0;
}

int open(const char* pathname, int flags) {

    return 0;
}

int open3(const char* pathname, int flags, mode_t mode) {

    return 0;
}

void FD_CLR(int fd, fd_set* fdset) {

}

int  FD_ISSET(int fd, fd_set* fdset) {

    return 0;
}

void FD_SET(int fd, fd_set* fdset) {
}

void FD_ZERO(fd_set* fdset) {

}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) {

    return 0;
}

int io_destroy(io_context_t ctx) {

    return 0;
}

int io_setup(int maxevents, io_context_t* ctxp) {

    return 0;
}

int eventfd(unsigned int initval, int flags) {

    return 0;
}

int io_submit(io_context_t ctx, long nr, struct iocb* ios[]) {

    return 0;    
}

int io_cancel(io_context_t ctx, struct iocb* iocb, struct io_event* evt) {

    return 0;
}

int io_getevents(io_context_t ctx, long min_nr, long nr, struct io_event* events, struct timespec* timeout) {

    return 0;
}

#ifdef __cplusplus 
}
#endif
