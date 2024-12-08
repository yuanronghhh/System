#include <System/Type/SysBlockPrivate.h>

SysPointer sys_block_malloc(SysSize size) {

  return sys_real_block_malloc(size);
}

void sys_block_free(SysPointer  self) {

 sys_real_block_free(self);
}

SysBool sys_block_valid_check(SysPointer self) {

  return sys_real_block_valid_check(self);
}

SysBool sys_block_check(SysPointer self) {

  return sys_real_block_check(self);
}

SysPointer sys_block_ref(SysPointer  self) {

  return sys_real_block_ref(self);
}

void sys_block_unref(SysPointer  self) {

  sys_real_block_unref(self);
}

void sys_block_ref_init(SysPointer self) {

  sys_real_block_ref_init(self);
}

void sys_block_ref_inc(SysPointer self) {

  sys_real_block_ref_inc(self);
}

SysRef sys_block_ref_get(SysPointer self) {

  return sys_real_block_ref_get(self);
}

SysBool sys_block_ref_dec(SysPointer self) {

  return sys_real_block_ref_dec(self);
}

SysBool sys_block_ref_cmp(SysPointer self, SysRef n) {

  return sys_real_block_ref_cmp(self, n);
}

void sys_block_ref_set(SysPointer self, SysRef n) {

  sys_real_block_ref_set(self, n);
}
