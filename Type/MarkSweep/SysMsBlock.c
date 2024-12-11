#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/Ref/SysRefBlock.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

SysPointer _sys_ms_block_cast_check(SysPointer o) {
  SysMsBlock *self = SYS_MS_BLOCK_B_CAST(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), NULL);

  return self;
}

static void sys_ms_block_create(SysMsBlock *o) {
  o->status = SYS_MS_STATUS_MALLOCED;
  o->type = SYS_MS_TRACK_MANUAL;

  sys_hlist_init((SysHList *)o);
}

SysPointer sys_ms_block_malloc(SysSize size) {
  SysMsBlock *o;

  o = ms_malloc0(MS_BSIZE + size);

  sys_ms_block_create(o);
  sys_ms_block_prepend(o);

  SysPointer bptr = SYS_MS_BLOCK_F_CAST(o);

  return bptr;
}

SysPointer sys_ms_block_realloc(SysPointer b, SysSize nsize) {
  SysMsBlock *o;

  o = ms_realloc(b, MS_BSIZE + nsize);
  sys_ms_block_create(o);
  sys_ms_block_prepend(o);

  return SYS_MS_BLOCK_F_CAST(o);
}

void sys_ms_block_free(void* o) {
  SysMsBlock *self = SYS_MS_BLOCK(o);

  if(self->type == SYS_MS_TRACK_AUTO) {

    sys_warning_N("%s", "Can not free managed block.");
    return;
  }

  sys_ms_block_remove(self);
  ms_free(self);
}

SysBool sys_ms_block_check(SysPointer o) {
  SysMsBlock *self = SYS_MS_BLOCK_B_CAST(o);
  sys_return_val_if_fail(SYS_IS_HDATA(self), false);

  return true;
}
