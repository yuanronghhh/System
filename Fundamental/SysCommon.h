#ifndef __SYS_COMMON_H__
#define __SYS_COMMON_H__

#include <SysConfig.h>

#ifdef  __cplusplus
#define SYS_BEGIN_DECLS  extern "C" {
#define SYS_END_DECLS    }
#else
#define SYS_BEGIN_DECLS
#define SYS_END_DECLS
#endif

#define _USE_MATH_DEFINES

#if _MSC_VER
  #include <Platform/Win32/SysWin32.h>
#elif __GNUC__
  #include <Platform/Unix/SysUnix.h>
#endif

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

#define UNUSED(o) ((void)o)
#define POINTER_TO_UINT(o) ((SysUInt)(SysUInt64)(o))
#define UINT_TO_POINTER(o) ((SysPointer)(SysUInt64)(o))
#define SYS_INLINE inline
#define SysBool bool
typedef int SysRef;
typedef void* SysPointer;
typedef int SysInt;
typedef unsigned int SysUInt;
typedef float SysFloat;
typedef char SysChar;
typedef unsigned char u_char;
typedef unsigned char SysUChar;
typedef unsigned char SysUInt8;
typedef unsigned int SysUInt32;
typedef signed long long SysInt64;
typedef unsigned long long SysUInt64;
typedef double SysDouble;
typedef size_t SysSize;
typedef wchar_t SysWChar;
#define SysNULL NULL

#define SYS_ARGS_FIXED(func, ptr) __FILE__, __func__, __LINE__, #func, #ptr,
#define SYS_ARGS_N_FIXED const SysChar *_filename, const SysChar *_funcname, const SysInt _line, const SysChar * _callfunc, const SysChar *_ptrstr,
#define SYS_ARGS_M_FIXED(func, ptr) SYS_ARGS_FIXED(func, ptr)
#define SYS_ARGS_P_FIXED(func, ptr) _filename, _funcname, _line, _callfunc, _ptrstr,

typedef void (*SysFunc) (SysPointer data, SysPointer user_data);
typedef int(*SysCompareFunc) (const void* a, const void* b);
typedef int(*SysCompareDataFunc) (const void* a, const void* b, SysPointer user_data);
typedef bool (*SysEqualFunc) (const SysPointer a, const SysPointer b);
typedef int(*SysEqualDataFunc) (const void* a, const void* b, void *user_data);
typedef void(*SysDestroyFunc) (void* data);
typedef SysPointer (*SysCopyFunc) (const SysPointer src, SysPointer data);

#endif
