#ifndef __SYS_BLOCK_H__
#define __SYS_BLOCK_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_BLOCK(o) ((SysBlock *)o)
#define SYS_IS_BLOCK(o) (sys_block_check((SysBlock *)o))

SysPointer sys_block_malloc(SysSize size);
void sys_block_free(SysPointer self);

SysBool sys_block_check(SysPointer self);
SysBool sys_block_valid_check(SysPointer self);

SYS_END_DECLS

#endif
