#ifndef a1
#define a1

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);

#define SIG_IGN    nullptr

#define SIGPIPE   100
#define SIGTERM   102

typedef struct sigset_t {
   int a;
}  sigset_t;


#ifdef __cplusplus
}
#endif


#endif

