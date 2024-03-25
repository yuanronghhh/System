#ifndef __SYS_BHEAP_H__
#define __SYS_BHEAP_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysBHeap SysBHeap;
typedef struct _SysBHeapIter SysBHeapIter;
typedef SysDouble (*SysBHeapFunc)(SysPointer node);

struct _SysBHeap {
  SysHArray array;
  SysBHeapFunc score_func;
};

struct _SysBHeapIter {
  /* <private> */
  SysUInt position;
  SysUInt len;
  SysPointer pdata[1];
};

SYS_API void sys_bheap_init(SysBHeap *hp, SysBHeapFunc func, SysDestroyFunc node_free);
SYS_API SysBHeap *sys_bheap_new(SysBHeapFunc func, SysDestroyFunc node_free);
SYS_API void sys_bheap_free(SysBHeap *hp, SysBool free_segment);
SYS_API void sys_bheap_destroy(SysBHeap *hp);

SYS_API SysBHeapIter *sys_bheap_iter_new(SysBHeap *hp);
SYS_API SysBool sys_bheap_iter_prev(SysBHeapIter* iter, SysPointer* data);
SYS_API SysBool sys_bheap_iter_next(SysBHeapIter* iter, SysPointer *data);
SYS_API void sys_bheap_iter_free(SysBHeapIter* iter);

SYS_API void sys_bheap_push(SysBHeap *hp, SysPointer node);
SYS_API SysPointer sys_bheap_peek(SysBHeap *hp);
SYS_API SysBool sys_bheap_remove(SysBHeap *hp, SysPointer node);
SYS_API SysInt sys_bheap_size(SysBHeap *hp);
SYS_API SysPointer sys_bheap_pop(SysBHeap *hp);
SYS_API SysHArray *sys_bheap_array(SysBHeap *hp);
SYS_API SysPointer sys_bheap_parent(SysBHeap *hp, SysPointer node);

SYS_END_DECLS

#endif
