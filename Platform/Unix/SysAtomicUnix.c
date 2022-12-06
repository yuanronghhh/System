#include <Platform/SysAtomicPrivate.h>

void sys_real_atomic_int_inc(SysInt *x) {
  __sync_fetch_and_add(x, 1);
}

bool sys_real_atomic_int_dec_and_test(SysInt *x) {
  return (__sync_fetch_and_add(x, -1) == 1);
}

bool sys_real_atomic_int_dec(SysInt *x) {
  return sys_real_atomic_int_dec_and_test(x);
}

bool sys_real_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n) {
  return __sync_bool_compare_and_swap(x, o, n);
}

bool sys_real_atomic_ptr_cmpxchg(SysPointer *x, SysPointer o, SysPointer n) {
  return __sync_bool_compare_and_swap (x, o, n);
}
