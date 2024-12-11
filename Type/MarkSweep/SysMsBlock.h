#ifndef __SYS_MS_BLOCK_H__
#define __SYS_MS_BLOCK_H__

#include <System/Type/SysBlock.h>

SYS_BEGIN_DECLS

#define SYS_MS_BLOCK(o) ((SysMsBlock *)sys_ms_block_cast_check(o))
#define SYS_IS_MS_BLOCK(o) (sys_ms_block_check((SysBlock *)o))

SysPointer sys_ms_block_malloc(SysSize size);
SysPointer sys_ms_block_realloc(SysPointer b, SysSize nsize);
void sys_ms_block_free(void* o);

SysBool sys_ms_block_check(SysPointer o);
SysPointer sys_ms_block_cast_check(SysPointer o);
SysMsBlock* sys_ms_block_b_cast(SysPointer o);
SysPointer sys_ms_block_f_cast(SysPointer o);
SysBool sys_ms_block_need_sweep(SysMsBlock *self);
SysBool sys_ms_block_need_mark(SysMsBlock *self);

void sys_ms_block_set_status(SysMsBlock *self, SysInt status);
SysInt sys_ms_block_get_status(SysMsBlock *self);

void sys_ms_block_set_type(SysMsBlock *self, SysInt type);
SysInt sys_ms_block_get_type(SysMsBlock *self);

SYS_END_DECLS

#endif
