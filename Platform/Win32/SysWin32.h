#ifndef __SYS_WIN32__
#define __SYS_WIN32__

#include <Platform/Win32/dirent.h>
#include <Psapi.h>
#include <time.h>
#include <basetsd.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>

#define SYS_OS_WIN32 1
#define SYS_API __declspec(dllexport)
#define PATH_SEP '\\'
typedef SSIZE_T ssize_t;
#define fileno _fileno

#if SYS_DEBUG
  #define _DEBUG 1
  #define VLD_IGNORE(code)      \
VLDGlobalDisable();             \
code                            \
VLDRestore();

  #include "vld.h"
#else
#endif

#endif
