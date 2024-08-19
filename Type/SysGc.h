#ifndef __SYS_GC_H__
#define __SYS_GC_H__

#include <System/Type/SysTypeCommon.h>

SYS_BEGIN_DECLS

#define SGC_BLOCK(o) ((SgcBlock *)_sys_block_cast_check((SgcBlock *)o))
#define sgc_type_e(var, tp, dst) do { \
  (var) = SGC_BLOCK(dst); \
  sgc_set((SysPointer *)&(var), tp, dst); \
} while(0)

#define sgc_stack_set(var, dst) sgc_type_e(var, SGC_IN_STACK, dst)
#define sgc_heap_set(var, dst) sgc_type_e(var, SGC_IN_HEAP, dst)

#define sys_type_from_sgc_block(o) (((SgcBlock *)o)->type)

#if USE_SGC_REF_COUNT
#define sgc_block_ref_init(o) sys_ref_count_init((SgcBlock *)o)
#define sgc_block_ref_check(o, max) SYS_REF_CHECK((SgcBlock *)o, max)
#define sgc_block_ref_valid_check(o, max) SYS_REF_VALID_CHECK((SgcBlock *)o, max)
#define sgc_block_ref_get(o) sys_ref_count_get((SgcBlock *)o)
#define sgc_block_ref_inc(o) sys_ref_count_inc((SgcBlock *)o)
#define sgc_block_ref_dec(o) sys_ref_count_dec((SgcBlock *)o)
#define sgc_block_ref_cmp(o, n) sys_ref_count_cmp((SgcBlock *)o, n)
#else
#define sgc_block_ref_init(o)
#define sgc_block_ref_check(o, max) true
#define sgc_block_ref_valid_check(o, max) true
#define sgc_block_ref_get(o) 0
#define sgc_block_ref_inc(o) true
#define sgc_block_ref_dec(o) true
#define sgc_block_ref_cmp(o, n) true
#endif

#define sgc_cleanup_attr(func) __attribute__((cleanup(func)))
#define sgc_autoptr(TypeName) sgc_cleanup_attr(sgc_cleanup) TypeName*
#define sgc_move(dst, src) _sgc_move((SgcBlock *)dst, (SgcBlock *)src, (SgcBlock **)&(dst), (SgcBlock **)&(src))

struct _SgcBlock {
  SysType type;
  /* <private> */
  // SysRef ref_count;
  SysPointer addr;
};

struct _SgcArea {
  SysPointer last_addr;
  SgcBlock* block;
  SysBool isstack;
};

#define sgc_block_create(o, type, ...) _sgc_block_create((SgcBlock *)(o), type, __VA_ARGS__)
#define sgc_block_destroy(o) _sgc_block_destroy((SgcBlock *)(o))

#define sgc_block_ref(o) _sgc_block_ref(SGC_BLOCK(o))
#define sgc_block_unref(o) _sgc_block_unref((SgcBlock *)(o))
SYS_API SysPointer _sgc_block_ref(SgcBlock* self);
SYS_API SysBool _sgc_block_unref(SgcBlock* self);

SYS_API void sgc_setup(void);
SYS_API void sgc_teardown(void);

SYS_API void sgc_block_set_unref_hook(SysRefHook hook);
SYS_API void sgc_block_set_ref_hook(SysRefHook hook);
SYS_API void sgc_block_set_new_hook(SysRefHook hook);

SYS_API void sgc_run(SgcCollector *gc);
SYS_API void sgc_stop(SgcCollector *gc);

SYS_API SysPointer sgc_malloc0(SysSize size);
SYS_API void sgc_free(SysPointer ptr);

#define sgc_type_new(type, count) _sgc_type_new(type, count)
SYS_API SysPointer _sgc_type_new(SysType type, SysSize count);
SYS_API void sgc_type_free(SysPointer ptr);
SYS_API void sgc_set(SysPointer *addr, SysInt isstack, SgcBlock *block);

SYS_API SysPointer _sys_block_cast_check(SgcBlock *self);
SYS_API SysBool _sgc_block_create(SgcBlock *o,
    SysType type,
    const SysChar * first,
    ...);
SYS_API SysBool _sgc_block_destroy(SgcBlock* self);
SYS_API SgcBlock* sgc_block_malloc0(SysType type, SysSize size);
SYS_API void* sgc_block_new(SysType type, const SysChar * first, ...);
#define sgc_block_free(o) _sgc_block_free((SgcBlock*)(o))
SYS_API void _sgc_block_free(SgcBlock* self);

/* auto gc */
#define SGC_DEFINE_AUTOPTR_CLEAN_FUNC(TypeName) \
  typedef TypeName* TypeName##_autoptr;

SGC_DEFINE_AUTOPTR_CLEAN_FUNC(SysChar)
SGC_DEFINE_AUTOPTR_CLEAN_FUNC(SysDouble)
SGC_DEFINE_AUTOPTR_CLEAN_FUNC(SysInt)
SGC_DEFINE_AUTOPTR_CLEAN_FUNC(SysPointer)

void _sgc_move(SgcBlock *dst,
    SgcBlock *src,
    SgcBlock **dst_addr,
    SgcBlock **src_addr);

void sgc_cleanup(void *b);

SYS_END_DECLS

#endif
