#ifndef __SYS_MEM_MAP_H__
#define __SYS_MEM_MAP_H__

#include <System/Platform/Common/SysMem.h>

#define malloc(size) sys_malloc0(size)
#define free(ptr) sys_free(ptr)
#define realloc(ptr, size) sys_realloc(ptr, size)

#endif
