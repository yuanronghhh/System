#ifndef __SYS_ANDROID__
#define __SYS_ANDROID__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <features.h>
#include <signal.h>
#include <spawn.h>
#include <ctype.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <execinfo.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <malloc.h>

#define PATH_SEP "/"
#ifndef MAX_PATH
  #define MAX_PATH 260
#endif

#define SYS_HANDLE SysInt
#define SYS_OS_ANDROID 1
#define SYS_API extern __attribute__((visibility("default")))

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define asb(a)     (((a) < 0) ? -(a) : (a))
#define clamp(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef __cplusplus
}
#endif

#endif

