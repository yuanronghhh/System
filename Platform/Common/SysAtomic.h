#ifndef __SYS_ATOMIC_H__
#define __SYS_ATOMIC_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#define sys_atomic_int_get(x) _sys_atomic_int_get((const volatile SysInt *)(x))
#define sys_atomic_int_set(x, n) _sys_atomic_int_set((volatile SysInt *)(x), n)

SYS_API SysInt _sys_atomic_int_get(const volatile SysInt *x);
SYS_API void _sys_atomic_int_set(volatile SysInt *x, SysInt n);
SYS_API void sys_atomic_int_inc(SysInt *x);
SYS_API SysBool sys_atomic_int_dec(SysInt *x);
SYS_API SysBool sys_atomic_int_dec_and_test(SysInt *x);
SYS_API SysBool sys_atomic_int_cmpxchg(SysInt *x, SysInt o, SysInt n);
#define sys_atomic_cmpxchg(x, o, n) sys_atomic_int_cmpxchg(x, o, n)
SYS_API SysBool sys_atomic_pointer_cmpxchg(volatile SysPointer* x, SysPointer o, SysPointer n);
SYS_API SysPointer sys_atomic_pointer_get(const volatile SysPointer x);
SYS_API void sys_atomic_pointer_set(SysPointer o, SysPointer n);

SYS_END_DECLS

#endif
