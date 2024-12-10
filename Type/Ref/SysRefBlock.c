#include <System/Type/Ref/SysRefBlock.h>
#include <System/Platform/Common/SysRefCountPrivate.h>
#include <System/Type/SysBlockPrivate.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Utils/SysError.h>
#include <System/DataTypes/SysHCommon.h>

#define SYS_REF_BLOCK_CAST(o) _sys_hdata_b_cast(o, sizeof(SysRefBlock))

SysPointer _sys_ref_block_cast_check(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK_CAST(o);
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p,%d", self->ref_count);
    return false;
  }

  return self;
}

SysBool sys_ref_block_create(SysRefBlock* self) {
  sys_ref_count_init(self);
  return true;
}

SysPointer sys_ref_block_malloc(SysSize size) {
  SysInt bsize = sizeof(SysRefBlock);
  SysRefBlock* o = ms_malloc0(bsize + size);

  sys_ref_block_create(o);

  return (SysChar *)o + bsize;
}

SysPointer sys_ref_block_realloc(SysPointer b, SysSize size) {
  SysInt bsize = sizeof(SysRefBlock);
  SysRefBlock* o = ms_realloc(b, bsize + size);

  sys_ref_block_create(o);

  return (SysChar *)o + bsize;
}

void _sys_ref_block_destroy(SysRefBlock* self) {

  sys_ref_count_set(self, 0);
}

void sys_ref_block_unref(SysPointer o) {
  SysRefBlock* self = SYS_REF_BLOCK(o);
  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return;
  }

  if(!sys_ref_count_dec(self)) {
    return;
  }

  ms_free(self);
}

SysPointer sys_ref_block_ref(SysPointer o) {
  SysRefBlock* self = SYS_REF_BLOCK(o);

  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return NULL;
  }

  sys_ref_count_inc(self);

  return SYS_BLOCK(self);
}

void sys_ref_block_free(SysPointer o) {
  SysRefBlock* self = SYS_REF_BLOCK(o);
  ms_free(self);
}

SysBool sys_ref_block_valid_check(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK_CAST(o);

  return sys_ref_count_valid_check(self, MAX_REF_NODE);
}

SysBool sys_ref_block_check(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK_CAST(o);

  return sys_ref_count_check(self, MAX_REF_NODE);
}

void sys_ref_block_ref_init(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK(o);
  sys_ref_count_init(self);
}

void sys_ref_block_ref_inc(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK(o);
  sys_ref_count_init(self);
}

SysRef sys_ref_block_ref_get(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK(o);
  return sys_ref_count_get(self);
}

SysBool sys_ref_block_ref_dec(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK(o);
  return sys_ref_count_dec(self);
}

SysBool sys_ref_block_ref_cmp(SysPointer o, SysRef n) {
  SysRefBlock *self = SYS_REF_BLOCK(o);

  return sys_ref_count_cmp(self, n);
}

void sys_ref_block_ref_set(SysPointer o, SysRef n) {
  SysRefBlock *self = SYS_REF_BLOCK(o);

  sys_ref_count_set(self, n);
}
