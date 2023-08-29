#ifndef __SYS_COMMON_H__
#define __SYS_COMMON_H__

#include <System/SysConfig.h>

#ifdef  __cplusplus
#define SYS_BEGIN_DECLS  extern "C" {
#define SYS_END_DECLS    }
#else
#define SYS_BEGIN_DECLS
#define SYS_END_DECLS
#endif

#define _USE_MATH_DEFINES

#if _MSC_VER
  #include <System/Platform/Win32/SysWin32.h>
#elif __GNUC__
  #include <System/Platform/Unix/SysUnix.h>
#endif

#include <stdint.h>
#include <limits.h>
#include <wchar.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>

#define UNUSED(o) ((void)o)
#define SYS_INLINE inline
typedef bool SysBool;
typedef int SysRef;
typedef void* SysPointer;
typedef int SysInt;
typedef unsigned short SysUShort;
typedef unsigned int SysUInt;
typedef float SysFloat;
typedef char SysChar;
typedef unsigned char u_char;
typedef unsigned char SysUChar;
typedef unsigned char SysUInt8;
typedef signed char SysInt8;
typedef signed short SysInt16;
typedef unsigned int SysUInt32;
typedef unsigned long SysULong;
typedef signed long long SysInt64;
typedef unsigned long long SysUInt64;
typedef double SysDouble;
typedef size_t SysSize;
typedef ssize_t SysSSize;
typedef wchar_t SysWChar;

#define SYS_STMT_START
#define SYS_STMT_END
#define SYS_UNLIKELY
#define SYS_LIKELY
#define SYS_ARGS(func, ptr) __FILE__, __func__, __LINE__, #func, #ptr,
#define SYS_ARGS_N const SysChar *_filename, const SysChar *_funcname, const SysInt _line, const SysChar * _callfunc, const SysChar *_ptrstr,
#define SYS_ARGS_M(func, ptr) SYS_ARGS(func, ptr)
#define SYS_ARGS_P(func, ptr) _filename, _funcname, _line, _callfunc, _ptrstr,

typedef void       (*SysFunc) (SysPointer data, SysPointer user_data);
typedef SysInt     (*SysCompareFunc) (const void* a, const void* b);
typedef SysInt     (*SysCompareDataFunc) (const void* a, const void* b, SysPointer user_data);
typedef SysBool    (*SysEqualFunc) (const SysPointer a, const SysPointer b);
typedef SysInt     (*SysEqualDataFunc) (const void* a, const void* b, void *user_data);
typedef void       (*SysDestroyFunc) (void* data);
typedef SysPointer (*SysCopyFunc) (const SysPointer src, SysPointer data);


#endif
