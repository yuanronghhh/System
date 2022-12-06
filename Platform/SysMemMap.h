#ifndef __SYS_MEM_MAP_H__
#define __SYS_MEM_MAP_H__

#include <Platform/SysMem.h>

#define malloc(size) sys_malloc0_N(size)
#define free(ptr) sys_free_N(ptr)
#define realloc(ptr, size) sys_realloc_N(ptr, size)

#endif
