#ifndef __SYS_MEM_H__
#define __SYS_MEM_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#if defined(USE_DEBUGGER) && SYS_OS_WIN32
  #if !defined(_DEBUG)
    #define _DEBUG SYS_DEBUG
  #endif 
  #include "vld.h"
  #define SYS_LEAK_IGNORE_BEGIN VLDGlobalDisable()
  #define SYS_LEAK_IGNORE_END VLDGlobalEnable()
#else
  #define SYS_LEAK_IGNORE_BEGIN
  #define SYS_LEAK_IGNORE_END
#endif

#define sys_align_up(o, base) (((o)+ (base - 1)) & ~((base) - 1))
#define sys_align_down(o, base) ((o) & ~((base) - 1))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__cplusplus)
#define SYS_ALIGNOF(type) _Alignof (type)
#else
#define SYS_ALIGNOF(type) (offsetof (struct { SysChar a; type b; }, b)) 
#endif

#define sys_alloca(size)  alloca (size)

#define sys_new(struct_type, n_structs) (struct_type *)sys_malloc((n_structs) * sizeof(struct_type))
#define sys_new0(struct_type, n_structs) (struct_type *)sys_malloc0((n_structs) * sizeof(struct_type))
#define sys_renew(struct_type, ptr, n_structs) (struct_type *)sys_realloc(ptr, (n_structs) * sizeof(struct_type))

#define sys_slice_new(type) sys_new(type, 1)
#define sys_slice_new0(type) sys_new0(type, 1)

#define sys_slice_free1(type, ptr) sys_free(ptr)
#define sys_slice_free(type, ptr) sys_slice_free1(type, ptr)
#define sys_slice_free_chain(type, ptr, next) _sys_slice_free_chain(sizeof(type), ptr, offsetof(type, next))

#define sys_clear_pointer(pp, destroy) _sys_clear_pointer((void **)(pp), (SysDestroyFunc)destroy)

SYS_API void sys_memcpy(
  SysPointer  const dst,
  SysSize     const dst_size,
  void        const* const src,
  SysSize     const src_size);


SYS_API SysPointer sys_realloc(void *block, SysSize size);
SYS_API SysPointer sys_calloc(SysSize count, SysSize size);
SYS_API SysPointer sys_malloc(SysSize size);
SYS_API void sys_free(void *block);
SYS_API SysPointer sys_malloc0(SysSize size);
SYS_API SysPointer sys_memdup(const SysPointer mem, SysUInt byte_size);
SYS_API void _sys_slice_free_chain(SysSize type, SysPointer ptr, SysSize offset);
SYS_API void _sys_clear_pointer(void **pp, SysDestroyFunc destroy);
SYS_API SysSize sys_get_msize(void *block);
SYS_API SysPointer sys_aligned_malloc(SysSize align, SysSize size);
SYS_API void sys_aligned_free(void *ptr);

SYS_API void sys_leaks_setup(void);
SYS_API void sys_leaks_report(void);
SYS_API const SysChar* sys_leaks_get_file(void);
SYS_API void sys_leaks_set_file(const SysChar *leakfile);

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
