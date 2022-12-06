#ifndef __SYS_MEM_H__
#define __SYS_MEM_H__

#include <Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#if SYS_OS_WIN32 && SYS_DEBUG
#define SYS_LEAK_IGNORE(code) VLD_IGNORE(code)
#else
#define SYS_LEAK_IGNORE(code) code
#endif

#define sys_align_up(o, base) (((o)+ (base - 1)) & ~((base) - 1))
#define sys_align_down(o, base) ((o) & ~((base) - 1))

#define sys_realloc_N(ptr, size) sys_realloc(ptr, size)
#define sys_malloc_N(size) sys_malloc(size)
#define sys_malloc0_N(size) sys_malloc0(size)
#define sys_free_N(ptr) { sys_free(ptr); (ptr) = NULL; }
#define sys_memdup_N(mem, byte_size) sys_memdup(mem, byte_size)

#define sys_new_N(struct_type, n_structs) (struct_type *)sys_malloc_N((n_structs) * sizeof(struct_type))
#define sys_new0_N(struct_type, n_structs) (struct_type *)sys_malloc0_N((n_structs) * sizeof(struct_type))
#define sys_renew_N(struct_type, ptr, n_structs) (struct_type *)sys_realloc_N(ptr, (n_structs) * sizeof(struct_type))

#define sys_slice_new(type) sys_new_N(type, 1)
#define sys_slice_new0(type) sys_new0_N(type, 1)

#define sys_slice_free1(type, ptr) sys_free_N(ptr)
#define sys_slice_free(type, ptr) sys_slice_free1(type, ptr)
#define sys_slice_free_chain(type, ptr, next) _sys_slice_free_chain(sizeof(type), ptr, offsetof(type, next))

SYS_API void sys_memcpy(
  SysPointer  const dst,
  SysSize     const dst_size,
  void        const* const src,
  SysSize     const src_size);

SYS_API SysPointer sys_realloc(void *block, SysSize size);
SYS_API SysPointer sys_malloc(SysSize size);
SYS_API void sys_free(void *block);
SYS_API SysPointer sys_malloc0(SysSize size);
SYS_API SysPointer sys_memdup(const SysPointer mem, SysUInt byte_size);
SYS_API void _sys_slice_free_chain(SysSize type, SysPointer ptr, SysSize offset);

SYS_API void sys_leaks_init(void);
SYS_API void sys_leaks_report(void);

static inline SysPointer sys_steal_pointer (SysPointer pp) {
  SysPointer *ptr = (SysPointer *)pp;
  SysPointer ref;

  ref = *ptr;
  *ptr = NULL;

  return ref;
}

/* type safety */
#define sys_steal_pointer(pp) (0 ? (*(pp)) : (sys_steal_pointer) (pp))

SYS_END_DECLS

#endif
