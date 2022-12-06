#include <Platform/SysAtomicPrivate.h>

void sys_atomic_int_inc(SysInt *x) {
  sys_real_atomic_int_inc(x);
}

bool sys_atomic_int_dec_and_test(SysInt *x) {
  return sys_real_atomic_int_dec_and_test(x);
}

bool sys_atomic_int_dec(SysInt *x) {
  return sys_real_atomic_int_dec(x);
}

bool sys_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n) {
  return sys_real_atomic_cmpxchg(x, o, n);
}

bool sys_atomic_ptr_cmpxchg(SysPointer x, SysPointer o, SysPointer n) {
  return sys_real_atomic_ptr_cmpxchg(x, o, n);
}
