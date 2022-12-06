#ifndef __SYS_ATOMIC_PRIVATE_H__
#define __SYS_ATOMIC_PRIVATE_H__

#include <Platform/SysAtomic.h>

SYS_BEGIN_DECLS

void sys_real_atomic_int_inc(SysInt *x);
bool sys_real_atomic_int_dec(SysInt *x);
bool sys_real_atomic_int_dec_and_test(SysInt *x);
bool sys_real_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n);
bool sys_real_atomic_ptr_cmpxchg(SysPointer *x, SysPointer o, SysPointer n);

SYS_END_DECLS

#endif
