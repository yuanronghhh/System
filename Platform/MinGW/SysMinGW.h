#ifndef __SYS_MINGW__
#define __SYS_MINGW__

#ifdef __cplusplus
extern "C" {
#endif

#define _CRT_INTERNAL_NONSTDC_NAMES 1
#if defined(_WIN32_WINNT)
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#include <Psapi.h>
#include <time.h>
#include <basetsd.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <DbgHelp.h>
#include <intrin.h>
#include <winioctl.h>

#define SYS_OS_MINGW 1

#if !defined(USE_STATIC)
#define SYS_API __declspec(dllexport)
#endif

#define PATH_SEP '\\'
#define SYS_DIR_SEPARATOR PATH_SEP
typedef SSIZE_T ssize_t;
#define fileno _fileno
// #define __builtin_frame_address(x)  ((void)(x), _AddressOfReturnAddress())

#ifdef __cplusplus
}
#endif

#endif
