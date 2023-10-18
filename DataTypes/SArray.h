#ifndef __S_ARRAY_H__
#define __S_ARRAY_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SArray SPtrArray;
typedef struct _SArray SArray;

struct _SRealArray {
  SysUInt8* data;
  SysUInt   len;

  /* <private> */
  SysUInt   alloc;
  SysUInt   elt_size;
  SysUInt   zero_terminated : 1;
  SysUInt   clear : 1;
  SysRef ref_count;
  SysDestroyFunc clear_func;
};

struct _SArray {
  SysUInt8 *data;
  SysUInt   len;
};

SYS_API bool s_array_init(SArray *self, SysBool zero_terminated, SysBool clear_, SysUInt element_size);
SYS_API bool s_ptr_array_init_full(SPtrArray* self, SysUInt reserved_size, SysDestroyFunc element_free_func);

SYS_END_DECLS

#endif
