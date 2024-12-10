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
