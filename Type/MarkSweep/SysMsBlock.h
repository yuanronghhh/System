#ifndef __SYS_MS_BLOCK_H__
#define __SYS_MS_BLOCK_H__

#include <System/Type/SysBlock.h>

SYS_BEGIN_DECLS

#define SYS_MS_BLOCK(o) ((SysMsBlock *)_sys_ms_block_cast_check(o))
#define SYS_IS_MS_BLOCK(o) (sys_ms_block_check((SysBlock *)o))

#define MS_BSIZE sizeof(SysMsBlock)
#define SYS_MS_BLOCK_B_CAST(o) _sys_hdata_b_cast(o, MS_BSIZE)
#define SYS_MS_BLOCK_F_CAST(o) _sys_hdata_f_cast(o, MS_BSIZE)

struct _SysMsBlock {
  SysHList parent;
  /* <private> */
  SYS_MS_TRACK_ENUM type;
  SYS_MS_STATUS_ENUM status;
};

SysPointer sys_ms_block_malloc(SysSize size);
SysPointer sys_ms_block_realloc(SysPointer b, SysSize nsize);
void sys_ms_block_free(void* o);

SysBool sys_ms_block_check(SysPointer o);
SysPointer _sys_ms_block_cast_check(SysPointer o);

SYS_END_DECLS

#endif
