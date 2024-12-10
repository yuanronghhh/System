#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/SysBlockPrivate.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

SysMsBlock* _sys_ms_block_cast_check(SysPointer o) {
  SysMsBlock *self = SYS_MS_BLOCK_B_CAST(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), NULL);

  return self;
}

static void sys_ms_block_create(SysMsBlock *o) {
  SysHList *ms_list;

  o->status = SYS_MS_STATUS_MALLOCED;
  o->type = SYS_MS_TRACK_MANUAL;
  ms_list = SYS_HLIST(o);
  sys_hlist_init(ms_list);
}

SysPointer sys_real_block_malloc(SysSize size) {
  SysMsBlock *o;

  o = ms_malloc0(MS_BSIZE + size);
  sys_ms_block_create(o);
  sys_ms_block_prepend(SYS_HLIST(o));

  SysPointer bptr = SYS_MS_BLOCK_F_CAST(o);

  return bptr;
}

SysPointer sys_real_block_realloc(SysPointer b, SysSize nsize) {
  SysMsBlock *o;

  o = ms_realloc(b, MS_BSIZE + nsize);
  sys_ms_block_create(o);
  sys_ms_block_prepend(SYS_HLIST(o));

  return SYS_MS_BLOCK_F_CAST(o);
}

void sys_real_block_free(void* o) {
  SysMsBlock *self = SYS_MS_BLOCK(o);

  sys_ms_block_remove(self);
}

SysBool sys_real_block_valid_check(SysBlock *o) {
  SysMsBlock *self = SYS_MS_BLOCK(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), false);

  return true;
}

SysBool sys_real_block_check(SysBlock *o) {
  SysMsBlock *self = SYS_MS_BLOCK(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), false);

  return true;
}

SysPointer sys_real_block_ref(SysBlock* self) {
  return self;
}

void sys_real_block_unref(SysBlock* self) {
}

void sys_real_block_ref_init(SysBlock *self) {
}

void sys_real_block_ref_inc(SysBlock *self) {
}

SysRef sys_real_block_ref_get(SysBlock *self) {
  return 0;
}

SysBool sys_real_block_ref_dec(SysBlock *self) {
  return false;
}

SysBool sys_real_block_ref_cmp(SysBlock *self, SysRef n) {
  return false;
}

void sys_real_block_ref_set(SysBlock *self, SysRef n) {
}
