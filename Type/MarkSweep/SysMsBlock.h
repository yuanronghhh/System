#ifndef __SYS_MS_BLOCK_H__
#define __SYS_MS_BLOCK_H__

#include <System/Type/SysBlock.h>

SYS_BEGIN_DECLS

#define SYS_MS_BLOCK(o) ((SysMsBlock *)sys_ms_pointer_cast_check(o))
#define SYS_IS_MS_BLOCK(o) (sys_ms_pointer_check(o))

#define MS_BSIZE sizeof(SysMsBlock)
#define SYS_MS_BLOCK_B_CAST(o) _sys_hdata_b_cast(o, MS_BSIZE)
#define SYS_MS_BLOCK_F_CAST(o) _sys_hdata_f_cast(o, MS_BSIZE)

struct _SysMsBlock {
  SysHList parent;
  /* <private> */

  /* SYS_MS_TRACK_ENUM */
  SysUShort type;
  /* SYS_MS_STATUS_ENUM */
  SysUShort status;
};

SysBool sys_ms_pointer_check(SysPointer o);
SysPointer sys_ms_pointer_cast_check(SysPointer o);

SysMsBlock* sys_ms_block_malloc(SysSize size);
void sys_ms_block_free(SysMsBlock *self);
SysMsBlock* sys_ms_pointer_realloc(SysPointer b, SysSize nsize);

SysBool sys_ms_block_need_sweep(SysMsBlock *self);
SysBool sys_ms_block_need_mark(SysMsBlock *self);

void sys_ms_block_set_status(SysMsBlock *self, SysShort status);
SysShort sys_ms_block_get_status(SysMsBlock *self);

void sys_ms_block_set_type(SysMsBlock *self, SysShort type);
SysShort sys_ms_block_get_type(SysMsBlock *self);

SYS_END_DECLS

#endif
