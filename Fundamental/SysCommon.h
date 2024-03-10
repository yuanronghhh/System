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
typedef unsigned short SysUInt16;
typedef unsigned int SysUInt32;
typedef unsigned long SysULong;
typedef long SysLong;
typedef signed long long SysInt64;
typedef unsigned long long SysUInt64;
typedef uintptr_t SysUIntPtr;
typedef double SysDouble;
typedef size_t SysSize;
typedef ssize_t SysSSize;
typedef wchar_t SysWChar;

#define SYS_STMT_START
#define SYS_STMT_END
#define SYS_UNLIKELY
#define SYS_LIKELY
#define SYS_LOG_ARGS(func, ptr) __FILE__, __func__, __LINE__, #func, #ptr,
#define SYS_LOG_ARGS_N const SysChar *_filename, const SysChar *_funcname, const SysInt _line, const SysChar * _callfunc, const SysChar *_ptrstr,
#define SYS_LOG_ARGS_M(func, ptr) SYS_LOG_ARGS(func, ptr)
#define SYS_LOG_ARGS_P(func, ptr) _filename, _funcname, _line, _callfunc, _ptrstr,

#define SYS_STRUCT_MEMBER_P(struct_p, struct_offset)   \
  ((SysPointer) ((SysUInt8*) (struct_p) + (SysLong) (struct_offset)))
#define SYS_STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
  (*(member_type*)SYS_STRUCT_MEMBER_P((struct_p), (struct_offset)))

#define SYS_CONTAINER_OF(o, TypeName, member) ((TypeName *)((char *)o - offsetof(TypeName, member)))

typedef void       (*SysFunc) (SysPointer data, SysPointer user_data);
typedef SysInt     (*SysCompareFunc) (const void* a, const void* b);
typedef SysInt     (*SysCompareDataFunc) (const void* a, const void* b, SysPointer user_data);
typedef SysBool    (*SysEqualFunc) (const SysPointer a, const SysPointer b);
typedef SysInt     (*SysEqualDataFunc) (const void* a, const void* b, void *user_data);
typedef void       (*SysDestroyFunc) (void* data);
typedef SysPointer (*SysCopyFunc) (const SysPointer src, SysPointer data);

/* system types */
typedef enum
{
  SYS_TRAVERSE_LEAVES     = 1 << 0,
  SYS_TRAVERSE_NON_LEAVES = 1 << 1,
  SYS_TRAVERSE_ALL        = SYS_TRAVERSE_LEAVES | SYS_TRAVERSE_NON_LEAVES,
  SYS_TRAVERSE_MASK       = 0x03,
  SYS_TRAVERSE_LEAFS      = SYS_TRAVERSE_LEAVES,
  SYS_TRAVERSE_NON_LEAFS  = SYS_TRAVERSE_NON_LEAVES
} SysTraverseFlags;

typedef enum
{
  SYS_IN_ORDER,
  SYS_PRE_ORDER,
  SYS_POST_ORDER,
  SYS_LEVEL_ORDER
} SysTraverseType;

typedef struct _SysParam SysParam;
typedef struct _SysParamClass SysParamClass;

typedef struct _SysPtrArray SysPtrArray;
typedef struct _SysArray SysArray;
typedef struct _SysByteArray SysByteArray;
typedef struct _SysHArray SysHArray;

#endif
