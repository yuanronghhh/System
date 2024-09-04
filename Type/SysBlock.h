#ifndef __SYS_GC_H__
#define __SYS_GC_H__

#include <System/Type/SysTypeCommon.h>

SYS_BEGIN_DECLS

#define SYS_BLOCK(o) ((SysBlock *)_sys_block_cast_check((SysBlock *)o))
#define sys_type_from_sys_block(o) (((SysBlock *)o)->type)

struct _SysBlock {
  SysType type;
  /* <private> */
  SysRef ref_count;
};

#define sys_block_ref_valid_check(o) sys_ref_count_valid_check((SysBlock *)(o), MAX_REF_NODE)
#define sys_block_ref_check(o) sys_ref_count_check((SysBlock *)(o), MAX_REF_NODE)
#define sys_block_ref(o) _sys_block_ref((SysBlock *)(o))
#define sys_block_unref(o) _sys_block_unref((SysBlock *)(o))

SYS_API SysPointer _sys_block_ref(SysBlock* self);
SYS_API void _sys_block_unref(SysBlock* self);

SYS_API void sys_block_set_unref_hook(SysRefHook hook);
SYS_API void sys_block_set_ref_hook(SysRefHook hook);
SYS_API void sys_block_set_new_hook(SysRefHook hook);

SYS_API void sys_block_new_hook(SysBlock* self);
SYS_API void sys_block_unref_hook(SysBlock* self);
SYS_API void sys_block_ref_hook(SysBlock* self);

SYS_API SysPointer _sys_block_cast_check(SysBlock *self);
SYS_API SysBool sys_block_create(SysBlock* self, SysType type);
SYS_API void _sys_block_destroy(SysBlock* self);

SYS_API SysPointer sys_block_new(SysType tp, SysSize size);
SYS_API void sys_block_free(SysBlock* self);

SYS_END_DECLS

#endif
