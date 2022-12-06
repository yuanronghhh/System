#ifndef __SYS_UNIX__
#define __SYS_UNIX__

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#include <features.h>
#include <signal.h>
#include <spawn.h>
#include <ctype.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <libintl.h>
#include <X11/Xlib.h>

#define PATH_SEP "/"
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#define sys_handle int
#define SYS_OS_UNIX 1
#define SYS_API extern __attribute__((visibility("default")))

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define asb(a)	   (((a) < 0) ? -(a) : (a))
#define clamp(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef __cplusplus
}
#endif

#endif
