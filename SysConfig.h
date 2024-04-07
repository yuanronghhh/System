#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

typedef signed long long SysInt64;
typedef unsigned long long SysUInt64;

#define POINTER_TO_UINT(o) ((SysUInt)(SysUInt64)(o))
#define POINTER_TO_INT(o) ((SysInt)(SysUInt64)(o))
#define UINT_TO_POINTER(o) ((SysPointer)(SysUInt64)(o))
#define INT_TO_POINTER(o) ((SysPointer)(SysInt64)(o))
#define INT64_CONSTANT(val)	((val##LL))
#define UINT64_CONSTANT(val)	((val##ULL))


#define SIZEOF_INT 8
#define SYS_DEBUG 0

#endif
