#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/Ref/SysRefBlock.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

#define MS_BSIZE sizeof(SysMsBlock)
#define SYS_MS_BLOCK_B_CAST(o) _sys_hdata_b_cast(o, MS_BSIZE)
#define SYS_MS_BLOCK_F_CAST(o) _sys_hdata_f_cast(o, MS_BSIZE)

struct _SysMsBlock {
  SysHList parent;
  /* <private> */

  /* SYS_MS_TRACK_ENUM */
  SysInt type;
  /* SYS_MS_STATUS_ENUM */
  SysInt status;
};

SysMsBlock* sys_ms_block_b_cast(SysPointer o) {

  return SYS_MS_BLOCK_B_CAST(o);
}

SysPointer sys_ms_block_f_cast(SysPointer o) {

  return SYS_MS_BLOCK_F_CAST(o);
}

SysPointer sys_ms_block_cast_check(SysPointer o) {
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

SysBool sys_ms_block_need_mark(SysMsBlock *self) {

  return self->type == SYS_MS_TRACK_AUTO;
}

SysBool sys_ms_block_need_sweep(SysMsBlock *self) {
  SysPointer bptr;

  bptr = sys_ms_block_f_cast(self);
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

void sys_ms_block_set_status(SysMsBlock *self, SYS_MS_STATUS_ENUM status) {
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
