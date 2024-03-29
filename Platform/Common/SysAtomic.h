#ifndef __SYS_ATOMIC_H__
#define __SYS_ATOMIC_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#define MAX_REF_NODE (1 << 30)

#define sys_atomic_int_get(x) _sys_atomic_int_get((const volatile SysInt *)x)
#define sys_atomic_int_set(x, n) _sys_atomic_int_set((const volatile SysInt *)x, n)

SYS_API SysInt _sys_atomic_int_get(const volatile SysInt *x);
SYS_API void _sys_atomic_int_set(volatile SysInt *x, SysInt n);
SYS_API void sys_atomic_int_inc(SysInt *x);
SYS_API bool sys_atomic_int_dec(SysInt *x);
SYS_API bool sys_atomic_int_dec_and_test(SysInt *x);
SYS_API bool sys_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n);
SYS_API bool sys_atomic_ptr_cmpxchg(SysPointer x, SysPointer o, SysPointer n);
SYS_API SysPointer sys_atomic_ptr_get(const volatile SysPointer x);
SYS_API void sys_atomic_ptr_set(void* o, SysPointer n);

#define SYS_REF_INIT_VALUE 1
#define SYS_REF_CHECK(o, max_ref) \
  (_sys_atomic_int_get(&(o)->ref_count) >= 0 \
   && _sys_atomic_int_get(&(o)->ref_count) < max_ref)

#define SYS_REF_VALID_CHECK(o, max_ref) \
  (_sys_atomic_int_get(&(o)->ref_count) > 0 \
   && _sys_atomic_int_get(&(o)->ref_count) < max_ref)

#define sys_ref_count_init(o) ((o)->ref_count = (SYS_REF_INIT_VALUE))
#define sys_ref_count_inc(o) sys_atomic_int_inc(&((o)->ref_count))
#define sys_ref_count_dec(o) sys_atomic_int_dec_and_test(&((o)->ref_count))
#define sys_ref_count_cmp(o, n) (_sys_atomic_int_get(&((o)->ref_count)) == n)

SYS_END_DECLS

#endif
