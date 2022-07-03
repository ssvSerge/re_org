#ifndef __sys_select_h__
#define __sys_select_h__

#ifdef __cplusplus 
extern "C" {
#endif


typedef struct fd_set {
    int a;
}   fd_set;


int select ( int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout );

void FD_CLR    (int fd, fd_set* fdset);
int  FD_ISSET  (int fd, fd_set* fdset);
void FD_SET    (int fd, fd_set* fdset);
void FD_ZERO   (fd_set* fdset);

#ifdef __cplusplus 
}
#endif


#endif
