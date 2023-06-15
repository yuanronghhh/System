#ifndef __SYS_ATOMIC_PRIVATE_H__
#define __SYS_ATOMIC_PRIVATE_H__

#include <System/Platform/Common/SysAtomic.h>

SYS_BEGIN_DECLS


SYS_API SysInt sys_real_atomic_int_get(const volatile SysInt *i);
SYS_API void sys_real_atomic_int_set(volatile SysInt *x, SysInt n);
SYS_API void sys_real_atomic_int_inc(SysInt *x);
SYS_API bool sys_real_atomic_int_dec(SysInt *x);
SYS_API bool sys_real_atomic_int_dec_and_test(SysInt *x);
SYS_API bool sys_real_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n);
SYS_API bool sys_real_atomic_pointer_cmpxchg(SysPointer *x, SysPointer o, SysPointer n);
SYS_API SysPointer sys_real_atomic_pointer_get(const volatile SysPointer x);
SYS_API void sys_real_atomic_pointer_set(volatile SysPointer o, SysPointer n);

SYS_END_DECLS

#endif
