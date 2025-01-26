#ifndef __SYS_MS_COMMON_H__
#define __SYS_MS_COMMON_H__

#include <System/Type/SysTypeCommon.h>
#include <System/DataTypes/SysHQueue.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysRefCount.h>

SYS_BEGIN_DECLS

#define SYS_MS_INIT_VALUE UINT_TO_POINTER(0xCCCCCCCC)
#define MS_IS_NULL_OR_INIT(addr) (*(addr) == SYS_MS_INIT_VALUE || *(addr) == NULL)

#define ms_new(type) ms_malloc0(sizeof(type))
#define ms_realloc(o, size) realloc(o, size)
#define ms_malloc(size) malloc(size)
#define ms_free(o) free(o)
SysPointer ms_malloc0(SysSize size);

SYS_END_DECLS

#endif
