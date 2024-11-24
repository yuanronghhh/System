#include <System/Type/SysRefBlock.h>
#include <System/Type/SysObject.h>
#include <System/Type/SysType.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>
#include <System/DataTypes/SysHashTable.h>

SysPointer _sys_ref_block_cast_check(SysPointer o) {
  SysRefBlock *self = _sys_hdata_b_cast(o, sizeof(SysRefBlock));
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_check(self, MAX_REF_NODE)) {
    sys_warning_N("object ref check failed: %p", self);
    return false;
  }

  return self;
}

SysBool sys_ref_block_create(SysRefBlock* self) {
  sys_ref_count_init(self);

  return true;
}

void sys_ref_block_free(SysRefBlock* b) {
  sys_return_if_fail(b->ref_count == 0);

  sys_free(b);
}

SysPointer sys_ref_block_new(SysSize size) {
  SysInt bsize = sizeof(SysRefBlock);
  SysRefBlock* o = sys_malloc0(bsize + size);

  sys_ref_block_create(o);

  return (SysChar *)o + bsize;
}

void _sys_ref_block_destroy(SysRefBlock* self) {

  sys_ref_count_set(self, 0);
}

void _sys_ref_block_unref(SysRefBlock* self) {
  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return;
  }

  if(!sys_ref_count_dec(self)) {
    return;
  }

  sys_ref_block_free(self);
}

SysPointer _sys_ref_block_ref(SysRefBlock* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return NULL;
  }

  sys_ref_count_inc(self);

  return (SysPointer)self;
}

void _sys_ref_block_free(SysRefBlock* self) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(sys_ref_count_check(self, MAX_REF_NODE));

  sys_free(self);
}
