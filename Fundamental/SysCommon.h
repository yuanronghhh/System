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

#if defined(_MSC_VER)
  #include <System/Platform/Win32/SysWin32.h>
#elif defined(__MSYS__)
  #include <System/Platform/MinGW/SysMinGW.h>
#elif defined(__ANDROID__)
  #include <System/Platform/Android/SysAndroid.h>
#elif defined(__UNIX__)
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
typedef unsigned short SysUInt16;
typedef unsigned int SysUInt32;
typedef unsigned long SysULong;
typedef long SysLong;
typedef uintptr_t SysUIntPtr;
typedef double SysDouble;
typedef size_t SysSize;
typedef ssize_t SysSSize;
typedef wchar_t SysWChar;

#define SYS_STMT_START do
#define SYS_STMT_END while(0)

#if defined(__GNUC__) && (__GNUC__ >= 3)
# define SYS_UNLIKELY(cond) (__builtin_expect ((cond), 0))
# define SYS_LIKELY(cond) (__builtin_expect ((cond), 1))
#else
# define SYS_UNLIKELY(cond) (cond)
# define SYS_LIKELY(cond) (cond)
#endif

#define SYS_LOG_ARGS(func, ptr) __FILE__, __func__, __LINE__, #func, #ptr,
#define SYS_LOG_ARGS_N const SysChar *_filename, const SysChar *_funcname, const SysInt _line, const SysChar * _callfunc, const SysChar *_ptrstr,
#define SYS_LOG_ARGS_M(func, ptr) SYS_LOG_ARGS(func, ptr)
#define SYS_LOG_ARGS_P _filename, _funcname, _line, _callfunc, _ptrstr,

#define SYS_STRUCT_MEMBER_P(struct_p, struct_offset)   \
  ((SysPointer) ((SysUInt8*) (struct_p) + (SysLong) (struct_offset)))
#define SYS_STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
  (*(member_type*)SYS_STRUCT_MEMBER_P((struct_p), (struct_offset)))

#define SYS_CONTAINER_OF(o, TypeName, member) ((TypeName *)((char *)o - offsetof(TypeName, member)))

#include <System/Fundamental/SysCommonTypes.h>

#endif
