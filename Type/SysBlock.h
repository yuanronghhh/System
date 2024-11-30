#ifndef __SYS_BLOCK_H__
#define __SYS_BLOCK_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_BLOCK(o) ((SysBlock *)o)
#define SYS_IS_BLOCK(o) (sys_block_check((SysBlock *)o))

#define sys_block_new(TypeName, size) sys_block_malloc((SysSize)size * sizeof(TypeName))
SysPointer sys_block_malloc(SysSize size);
void sys_block_free(SysPointer self);
SysPointer sys_block_ref(SysPointer self);
void sys_block_unref(SysPointer self);

SysBool sys_block_check(SysPointer self);
SysBool sys_block_valid_check(SysPointer self);
void sys_block_ref_count_init(SysPointer self);
void sys_block_ref_count_inc(SysPointer self);
SysRef sys_block_ref_count_get(SysPointer self);
SysBool sys_block_ref_count_dec(SysPointer self);
SysBool sys_block_ref_count_cmp(SysPointer self, SysRef n);
void sys_block_ref_count_set(SysPointer self, SysRef n);

SYS_END_DECLS

#endif
