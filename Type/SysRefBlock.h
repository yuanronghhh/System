#ifndef __SYS_REF_BLOCK_H__
#define __SYS_REF_BLOCK_H__

#include <System/Type/SysTypeCommon.h>

SYS_BEGIN_DECLS

#define SYS_REF_BLOCK(o) ((SysRefBlock *)_sys_ref_block_cast_check(o))
#define SYS_IS_REF_BLOCK(o) (_sys_ref_block_cast_check(o))

struct _SysRefBlock {
  /* <private> */
  SysRef ref_count;
};

#define sys_ref_block_ref_valid_check(o) sys_ref_count_valid_check(o, MAX_REF_NODE)
#define sys_ref_block_ref_check(o) sys_ref_count_check(o, MAX_REF_NODE)
#define sys_ref_block_ref(o) _sys_ref_block_ref(o)
#define sys_ref_block_unref(o) _sys_ref_block_unref(o)

SYS_API SysPointer _sys_ref_block_ref(SysRefBlock* self);
SYS_API void _sys_ref_block_unref(SysRefBlock* self);
SYS_API void _sys_ref_block_free(SysRefBlock* self);

SYS_API void sys_ref_block_set_unref_hook(SysRefHook hook);
SYS_API void sys_ref_block_set_ref_hook(SysRefHook hook);
SYS_API void sys_ref_block_set_new_hook(SysRefHook hook);

SYS_API SysPointer _sys_ref_block_cast_check(SysPointer o);
SYS_API SysBool sys_ref_block_create(SysRefBlock* self);
SYS_API void _sys_ref_block_destroy(SysRefBlock* self);

SYS_API SysPointer sys_ref_block_new(SysSize size);
SYS_API void sys_ref_block_free(SysRefBlock* self);

SYS_END_DECLS

#endif
