#ifndef __SYS_BHEAP_H__
#define __SYS_BHEAP_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysBHeap SysBHeap;
typedef SysDouble(*SysBHeapFunc)(SysPointer node);

SYS_API SysBHeap *sys_bheap_new(SysBHeapFunc func, SysDestroyFunc node_free);
SYS_API void sys_bheap_free(SysBHeap *hp, SysBool free_segment);

SYS_API void sys_bheap_push(SysBHeap *hp, SysPointer node);
SYS_API SysPointer sys_bheap_peek(SysBHeap *hp);
SYS_API SysBool sys_bheap_remove(SysBHeap *hp, SysPointer node);
SYS_API SysInt sys_bheap_size(SysBHeap *hp);
SYS_API SysPointer sys_bheap_pop(SysBHeap *hp);
SYS_API SysPtrArray *sys_bheap_array(SysBHeap *hp);

SYS_END_DECLS

#endif
