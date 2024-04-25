#ifndef __SYS_MINGW__
#define __SYS_MINGW__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

// #include <features.h>
// #include <spawn.h>
// #include <dlfcn.h>
// #include <sys/statfs.h>
// #include <libintl.h>
// #include <X11/Xlib.h>
// #include <execinfo.h>
// #include <sys/socket.h>
// #include <sys/ioctl.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <malloc.h>

#define PATH_SEP "/"
#ifndef MAX_PATH
  #define MAX_PATH 260
#endif

#define SYS_HANDLE int
#define SYS_OS_UNIX 1
#define SYS_API extern __attribute__((visibility("default")))

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define asb(a)     (((a) < 0) ? -(a) : (a))
#define clamp(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef __cplusplus
}
#endif

#endif
