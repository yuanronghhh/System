#ifndef __SYS_COMMON_TYPES_H__
#define __SYS_COMMON_TYPES_H__

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

typedef enum _SYS_IO_ENUM {
  SYS_IO_INVALID,
  SYS_IO_PIPE,
  SYS_IO_STDOUT,
  SYS_IO_DEV_NULL
} SYS_IO_ENUM;

typedef struct _SysPtrArray SysPtrArray;
typedef struct _SysArray SysArray;
typedef struct _SysByteArray SysByteArray;
typedef struct _SysHArray SysHArray;
typedef struct _SysValue SysValue;
typedef struct _SysSArg SysSArg;
typedef struct _SysElapse SysElapse;
typedef struct _SysSocket SysSocket;
typedef struct _SysSocketClass SysSocketClass;
typedef struct _SysSsl SysSsl;

#endif
