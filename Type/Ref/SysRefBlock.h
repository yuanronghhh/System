#ifndef __SYS_REF_BLOCK_H__
#define __SYS_REF_BLOCK_H__

#include <System/Type/SysBlock.h>

SYS_BEGIN_DECLS

#define SYS_REF_BLOCK(o) ((SysRefBlock *)_sys_ref_block_cast_check(o))
#define SYS_IS_REF_BLOCK(o) (sys_ref_block_check(o))

struct _SysRefBlock {
  /* <private> */
  SysRef ref_count;
};

#define sys_ref_block_new(TypeName, size) sys_ref_block_malloc((SysSize)size * sizeof(TypeName))

SYS_API SysBool sys_ref_block_valid_check(SysPointer o);
SYS_API SysBool sys_ref_block_check(SysPointer o);
SYS_API SysPointer _sys_ref_block_cast_check(SysPointer o);
SYS_API SysBool sys_ref_block_create(SysRefBlock* self);
SYS_API void _sys_ref_block_destroy(SysRefBlock* self);

SYS_API void sys_ref_block_free(SysRefBlock *self);
SYS_API SysPointer sys_ref_block_realloc(SysRefBlock *b, SysSize size);
SYS_API SysPointer sys_ref_block_malloc(SysSize size);

SYS_API SysPointer sys_ref_block_ref(SysRefBlock *self);
SYS_API void sys_ref_block_unref(SysRefBlock *self);

SYS_API void sys_set_unref_hook(SysRefHook hook);
SYS_API void sys_set_ref_hook(SysRefHook hook);
SYS_API void sys_set_new_hook(SysRefHook hook);

SYS_API void sys_ref_block_ref_init(SysRefBlock *self);
SYS_API void sys_ref_block_ref_inc(SysRefBlock *self);
SYS_API SysRef sys_ref_block_ref_get(SysRefBlock *self);
SYS_API SysBool sys_ref_block_ref_dec(SysRefBlock *self);
SYS_API SysBool sys_ref_block_ref_cmp(SysRefBlock *self, SysRef n);
SYS_API void sys_ref_block_ref_set(SysRefBlock *self, SysRef n);

SYS_END_DECLS

#endif
