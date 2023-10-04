#include <System/DataTypes/SArray.h>


bool s_array_init(SArray *self, SysBool zero_terminated, SysBool clear_, SysUInt element_size) {
  sys_return_val_if_fail(element_size > 0, false);

  return true;
}

SysBool s_ptr_array_init_full(SPtrArray* self, SysUInt reserved_size, SysDestroyFunc element_free_func) {
  return true;
}

