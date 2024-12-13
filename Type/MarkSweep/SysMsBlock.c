#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/Ref/SysRefBlock.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

static void sys_ms_block_create(SysMsBlock *o) {
  o->status = SYS_MS_STATUS_MALLOCED;
  o->type = SYS_MS_TRACK_MANUAL;

  sys_hlist_init((SysHList *)o);
}


SysPointer sys_ms_pointer_cast_check(SysPointer o) {
  SysMsBlock *self = SYS_MS_BLOCK_B_CAST(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), NULL);

  return self;
}

SysBool sys_ms_pointer_check(SysPointer o) {
  SysMsBlock *self = SYS_MS_BLOCK_B_CAST(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), false);

  return true;
}

SysMsBlock* sys_ms_pointer_realloc(SysPointer b, SysSize nsize) {
  SysMsBlock *o;

  o = ms_realloc(b, MS_BSIZE + nsize);
  sys_ms_block_create(o);

  return o;
}

SysMsBlock* sys_ms_block_malloc(SysSize size) {
  SysMsBlock *o;

  o = ms_malloc0(MS_BSIZE + size);

  sys_ms_block_create(o);

  return o;
}


SysBool sys_ms_block_need_mark(SysMsBlock *self) {

  return self->type == SYS_MS_TRACK_AUTO;
}

SysBool sys_ms_block_need_sweep(SysMsBlock *self) {
  SysPointer bptr;

  bptr = SYS_MS_BLOCK_F_CAST(self);
  if(bptr == NULL) {
    return false;
  }

  if(self->type != SYS_MS_TRACK_AUTO) {
    return false;
  }

  if(self->status != SYS_MS_STATUS_MALLOCED) {
    return false;
  }

  return true;
}

void sys_ms_block_set_status(SysMsBlock *self, SysInt status) {
  sys_return_if_fail(self != NULL);

  self->status = status;
}

SysInt sys_ms_block_get_status(SysMsBlock *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->status;
}

void sys_ms_block_set_type(SysMsBlock *self, SysInt type) {
  sys_return_if_fail(self != NULL);

  self->type = type;
}

SysInt sys_ms_block_get_type(SysMsBlock *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->type;
}

void sys_ms_block_free(SysMsBlock *self) {
  sys_return_if_fail(self != NULL);
  self->status = SYS_MS_STATUS_FREEED;
  ms_free(self);
}
