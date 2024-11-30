#ifndef __SYS_REF_COUNT_PRIVATE_H__
#define __SYS_REF_COUNT_PRIVATE_H__

#include <System/Platform/Common/SysAtomic.h>

SYS_BEGIN_DECLS

#define MAX_REF_NODE 0xffffff
#define SYS_REF_INIT_VALUE 1
#define sys_ref_count_check(o, max_ref) \
  (_sys_atomic_int_get(&(o)->ref_count) >= 0 \
   && _sys_atomic_int_get(&(o)->ref_count) < (max_ref))

#define sys_ref_count_valid_check(o, max_ref) \
  (_sys_atomic_int_get(&(o)->ref_count) > 0 \
   && _sys_atomic_int_get(&(o)->ref_count) < (max_ref))

#define sys_ref_count_init(o) ((o)->ref_count = (SYS_REF_INIT_VALUE))
#define sys_ref_count_inc(o) sys_atomic_int_inc(&((o)->ref_count))
#define sys_ref_count_get(o) sys_atomic_int_get(&((o)->ref_count))
#define sys_ref_count_dec(o) sys_atomic_int_dec_and_test(&((o)->ref_count))
#define sys_ref_count_cmp(o, n) (_sys_atomic_int_get(&((o)->ref_count)) == n)
#define sys_ref_count_set(o, n) _sys_atomic_int_set(&((o)->ref_count), 0)

SYS_END_DECLS

#endif
