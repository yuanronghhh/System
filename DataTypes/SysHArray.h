#ifndef __SYS_HARRAY_H__
#define __SYS_HARRAY_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

struct _SysHArray {
  SysPointer* pdata;
  SysUInt   len;

  /* <private> */
  SysUInt   alloc;
  SysUInt   elt_size;
  SysUInt   zero_terminated : 1;
  SysUInt   clear : 1;
  SysRef ref_count;
  SysDestroyFunc  element_free_func;
};

SYS_API SysHArray* sys_harray_new(void);
SYS_API void sys_harray_free(SysHArray* self, SysBool free_segment);
SYS_API void sys_harray_unref(SysHArray* self);
SYS_API SysHArray* sys_harray_ref(SysHArray *self);
SYS_API void sys_harray_copy(SysHArray* dst, SysHArray* src, SysCopyFunc elem_copy, SysPointer copy_user_data);
SYS_API void sys_harray_destroy(SysHArray *self);
SYS_API SysPointer sys_harray_steal_index(SysHArray* array, SysUInt index_);
SYS_API SysHArray* sys_harray_new_with_free_func(SysDestroyFunc element_free_func);
SYS_API void sys_harray_init(SysHArray *self);
SYS_API void sys_harray_init_full(SysHArray* self, SysUInt reserved_size, SysDestroyFunc element_free_func);
SYS_API void sys_harray_init_with_free_func(SysHArray* self, SysDestroyFunc element_free_func);
SYS_API void sys_harray_add(SysHArray* self, SysPointer data);
SYS_API void sys_harray_insert(SysHArray* self, SysInt index_, SysPointer data);

SYS_END_DECLS

#endif
