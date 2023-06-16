#include <System/Platform/Common/SysAtomicPrivate.h>

void sys_real_atomic_int_inc(SysInt *x) {
  InterlockedIncrement(x);
}

SysInt sys_real_atomic_int_get(const volatile SysInt *i) {
  MemoryBarrier();
  return *i;
}

void sys_real_atomic_int_set(volatile SysInt *x, SysInt n) {
  *x = n;
  MemoryBarrier();
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

SysPointer sys_real_atomic_ptr_get(const volatile SysPointer x) {
  const SysPointer *ptr = x;

  MemoryBarrier();
  return *ptr;
}

void sys_real_atomic_ptr_set(volatile void *o, SysPointer n) {
  volatile void **ptr = o;

  *ptr = n;
  MemoryBarrier();
}
