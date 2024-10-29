#ifndef __SYS_COMMON_TYPES_H__
#define __SYS_COMMON_TYPES_H__

#define SYSINT64_CONSTANT(val)    (val##L)
#define SYSUINT64_CONSTANT(val)    (val##UL)

#define SYSUINT16_SWAP_LE_BE_CONSTANT(val)    ((SysUInt16) ( \
    (SysUInt16) ((SysUInt16) (val) >> 8) |    \
    (SysUInt16) ((SysUInt16) (val) << 8)))

#define SYSUINT32_SWAP_LE_BE_CONSTANT(val)    ((SysUInt32) ( \
    (((SysUInt32) (val) & (SysUInt32) 0x000000ffU) << 24) | \
    (((SysUInt32) (val) & (SysUInt32) 0x0000ff00U) <<  8) | \
    (((SysUInt32) (val) & (SysUInt32) 0x00ff0000U) >>  8) | \
    (((SysUInt32) (val) & (SysUInt32) 0xff000000U) >> 24)))

#define SYSUINT64_SWAP_LE_BE_CONSTANT(val)    ((SysUInt64) ( \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x00000000000000ffU)) << 56) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x000000000000ff00U)) << 40) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x0000000000ff0000U)) << 24) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x00000000ff000000U)) <<  8) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x000000ff00000000U)) >>  8) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x0000ff0000000000U)) >> 24) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0x00ff000000000000U)) >> 40) |    \
      (((SysUInt64) (val) &                        \
    (SysUInt64) SYSINT64_CONSTANT (0xff00000000000000U)) >> 56)))

#define SYSUINT16_SWAP_LE_BE(val) (SYSUINT16_SWAP_LE_BE_CONSTANT (val))
#define SYSUINT32_SWAP_LE_BE(val) (SYSUINT32_SWAP_LE_BE_CONSTANT (val))
#define SYSUINT64_SWAP_LE_BE(val) (SYSUINT64_SWAP_LE_BE_CONSTANT (val))

#define SYSINT16_TO_LE(val)	((SysInt16) (val))
#define SYSUINT16_TO_LE(val)	((SysUInt16) (val))
#define SYSINT16_TO_BE(val)	((SysInt16) SYSUINT16_SWAP_LE_BE (val))
#define SYSUINT16_TO_BE(val)	(SYSUINT16_SWAP_LE_BE (val))

#define SYSINT32_TO_LE(val)	((SysInt32) (val))
#define SYSUINT32_TO_LE(val)	((SysUInt32) (val))
#define SYSINT32_TO_BE(val)	((SysInt32) SYSUINT32_SWAP_LE_BE (val))
#define SYSUINT32_TO_BE(val)	(SYSUINT32_SWAP_LE_BE (val))

#define SYSINT64_TO_LE(val)	((SysInt64) (val))
#define SYSUINT64_TO_LE(val)	((SysUInt64) (val))
#define SYSINT64_TO_BE(val)	((SysInt64) SYSUINT64_SWAP_LE_BE (val))
#define SYSUINT64_TO_BE(val)	(SYSUINT64_SWAP_LE_BE (val))

#define SYSLONSYS_TO_LE(val)	((SysLong) SYSINT64_TO_LE (val))
#define SYSULONSYS_TO_LE(val)	((SysULong) SYSUINT64_TO_LE (val))
#define SYSLONSYS_TO_BE(val)	((SysLong) SYSINT64_TO_BE (val))
#define SYSULONSYS_TO_BE(val)	((SysULong) SYSUINT64_TO_BE (val))
#define SYSINT_TO_LE(val)		((SysInt) SYSINT32_TO_LE (val))
#define SYSUINT_TO_LE(val)	((SysUInt) SYSUINT32_TO_LE (val))
#define SYSINT_TO_BE(val)		((SysInt) SYSINT32_TO_BE (val))
#define SYSUINT_TO_BE(val)	((SysUInt) SYSUINT32_TO_BE (val))
#define SYSSIZE_TO_LE(val)	((SysSize) SYSUINT64_TO_LE (val))
#define SYSSSIZE_TO_LE(val)	((SysSSize) SYSINT64_TO_LE (val))
#define SYSSIZE_TO_BE(val)	((SysSize) SYSUINT64_TO_BE (val))
#define SYSSSIZE_TO_BE(val)	((SysSSize) SYSINT64_TO_BE (val))
#define SYS_BYTE_ORDER SYS_LITTLE_ENDIAN

#define SYSINT16_FROM_LE(val)	(SYSINT16_TO_LE (val))
#define SYSUINT16_FROM_LE(val)	(SYSUINT16_TO_LE (val))
#define SYSINT16_FROM_BE(val)	(SYSINT16_TO_BE (val))
#define SYSUINT16_FROM_BE(val)	(SYSUINT16_TO_BE (val))
#define SYSINT32_FROM_LE(val)	(SYSINT32_TO_LE (val))
#define SYSUINT32_FROM_LE(val)	(SYSUINT32_TO_LE (val))
#define SYSINT32_FROM_BE(val)	(SYSINT32_TO_BE (val))
#define SYSUINT32_FROM_BE(val)	(SYSUINT32_TO_BE (val))

#define SYSINT64_FROM_LE(val)	(SYSINT64_TO_LE (val))
#define SYSUINT64_FROM_LE(val)	(SYSUINT64_TO_LE (val))
#define SYSINT64_FROM_BE(val)	(SYSINT64_TO_BE (val))
#define SYSUINT64_FROM_BE(val)	(SYSUINT64_TO_BE (val))

#define SYSLONSYS_FROM_LE(val)	(SYSLONSYS_TO_LE (val))
#define SYSULONSYS_FROM_LE(val)	(SYSULONSYS_TO_LE (val))
#define SYSLONSYS_FROM_BE(val)	(SYSLONSYS_TO_BE (val))
#define SYSULONSYS_FROM_BE(val)	(SYSULONSYS_TO_BE (val))

#define SYSINT_FROM_LE(val)	(SYSINT_TO_LE (val))
#define SYSUINT_FROM_LE(val)	(SYSUINT_TO_LE (val))
#define SYSINT_FROM_BE(val)	(SYSINT_TO_BE (val))
#define SYSUINT_FROM_BE(val)	(SYSUINT_TO_BE (val))

#define SYSSIZE_FROM_LE(val)	(SYSSIZE_TO_LE (val))
#define SYSSSIZE_FROM_LE(val)	(SYSSSIZE_TO_LE (val))
#define SYSSIZE_FROM_BE(val)	(SYSSIZE_TO_BE (val))
#define SYSSSIZE_FROM_BE(val)	(SYSSSIZE_TO_BE (val))

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

typedef enum
{
  SYS_TIME_TYPE_STANDARD,
  SYS_TIME_TYPE_DAYLIGHT,
  SYS_TIME_TYPE_UNIVERSAL
} SysTimeType;

typedef struct _TimeZoneRule TimeZoneRule;
typedef struct _SysTimeZone SysTimeZone;

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
typedef SysUInt32 SysQuark;

#endif
