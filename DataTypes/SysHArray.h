#ifndef __SYS_HARRAY_H__
#define __SYS_HARRAY_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysHArray SysHArray;

struct _SysHArray {
  SysPointer* pdata;
  SysUInt   len;

  /* <private> */
  SysUInt   alloc;
  SysUInt   elt_size;
  SysUInt   zero_terminated : 1;
  SysUInt   clear : 1;
  SysDestroyFunc  element_free_func;
};

SYS_API SysBool sys_harray_init(SysHArray *self);
SYS_API SysBool sys_harray_init_full(SysHArray* self, SysUInt reserved_size, SysDestroyFunc element_free_func);
SYS_API SysBool sys_harray_init_with_free_func(SysHArray* self, SysDestroyFunc element_free_func);
SYS_API void sys_harray_insert(SysHArray *self, SysInt index_, SysPointer data);

SYS_END_DECLS

#endif
