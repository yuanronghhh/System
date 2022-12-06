#include <Platform/SysAtomicPrivate.h>

void sys_real_atomic_int_inc(SysInt *x) {
  InterlockedIncrement(x);
}

bool sys_real_atomic_int_dec_and_test(SysInt *x) {
  return InterlockedDecrement(x) == 0;
}

bool sys_real_atomic_int_dec(SysInt *x) {
  return sys_real_atomic_int_dec_and_test(x);
}

bool sys_real_atomic_cmpxchg(SysInt *x, SysInt o, SysInt n) {
  return InterlockedCompareExchange(x, n, o) == o;
}

bool sys_real_atomic_ptr_cmpxchg(SysPointer *x, SysPointer o, SysPointer n) {
  return InterlockedCompareExchangePointer(x, n, o) == o;
}
