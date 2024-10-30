#ifndef __SYS_WIN32__
#define __SYS_WIN32__

#ifdef __cplusplus
extern "C" {
#endif

#include <System/Platform/Win32/dirent.h>
#include <shellapi.h>
#include <Psapi.h>
#include <time.h>
#include <basetsd.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <DbgHelp.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <shlobj_core.h>
#include <intrin.h>

#define SYS_OS_WIN32 1
#define SYS_API __declspec(dllexport)
#define PATH_SEP '\\'
#define SYS_DIR_SEPARATOR PATH_SEP
typedef SSIZE_T ssize_t;
#define fileno _fileno
#define __builtin_frame_address(x)  ((void)(x), _AddressOfReturnAddress())

#ifdef __cplusplus
}
#endif

#endif
