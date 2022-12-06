#ifndef __SYS_ATOMIC_H__
#define __SYS_ATOMIC_H__

#include <Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

SYS_API void sys_atomic_int_inc(SysInt *x);
SYS_API bool sys_atomic_int_dec(SysInt *x);
SYS_API bool sys_atomic_int_dec_and_test(SysInt *x);
SYS_API bool sys_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n);
SYS_API bool sys_atomic_ptr_cmpxchg(SysPointer x, SysPointer o, SysPointer n);

#define SYS_REF_INIT_VALUE 1
#define sys_ref_count_init(o) ((o)->ref_count = (SYS_REF_INIT_VALUE))
#define sys_ref_count_inc(o) sys_atomic_int_inc(&((o)->ref_count))
#define sys_ref_count_dec(o) sys_atomic_int_dec_and_test(&((o)->ref_count))
#define sys_ref_count_last(o) sys_atomic_cmpxchg(&((o)->ref_count), &((o)->ref_count), 1)

SYS_END_DECLS

#endif
