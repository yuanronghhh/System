#ifndef __SYS_BLOCK_PRIVATE_H__
#define __SYS_BLOCK_PRIVATE_H__

#include <System/Type/SysBlock.h>

SYS_BEGIN_DECLS

SysPointer sys_real_block_malloc(SysSize size);
void sys_real_block_free(void* o);
SysPointer sys_real_block_realloc (SysPointer b, SysSize size);

SysPointer sys_real_block_ref(SysBlock* self);
void sys_real_block_unref(SysBlock* self);
SysBool sys_real_block_check(SysBlock *o);
SysBool sys_real_block_valid_check(SysBlock *self);
void sys_real_block_ref_init(SysBlock *o);
void sys_real_block_ref_inc(SysBlock *o);
SysRef sys_real_block_ref_get(SysBlock *o);
SysBool sys_real_block_ref_dec(SysBlock *o);
SysBool sys_real_block_ref_cmp(SysBlock *o, SysRef n);
void sys_real_block_ref_set(SysBlock *o, SysRef n);

SYS_END_DECLS

#endif
