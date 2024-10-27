#include <System/Platform/Common/SysAtomic.h>

void sys_atomic_int_inc(SysInt *x) {
  InterlockedIncrement((volatile long *)x);
}

SysInt _sys_atomic_int_get(const volatile SysInt *i) {
  MemoryBarrier();
  return *i;
}

void _sys_atomic_int_set(volatile SysInt *x, SysInt n) {
  *x = n;
  MemoryBarrier();
}

SysBool sys_atomic_int_dec_and_test(SysInt *x) {
  return InterlockedDecrement((volatile long*)x) == 0;
}

SysBool sys_atomic_int_dec(SysInt *x) {
  return sys_atomic_int_dec_and_test(x);
}

SysBool sys_atomic_int_cmpxchg(SysInt *x, SysInt o, SysInt n) {
  return InterlockedCompareExchange((volatile long*)x, n, o) == o;
}

SysBool sys_atomic_pointer_cmpxchg(volatile SysPointer* x, SysPointer o, SysPointer n) {

  return InterlockedCompareExchangePointer(x, n, o) == o;
}

SysPointer sys_atomic_pointer_get(const volatile SysPointer x) {
  const SysPointer *ptr = x;

  MemoryBarrier();
  return *ptr;
}

void sys_atomic_pointer_set(volatile SysPointer o, SysPointer n) {
  volatile SysPointer *ptr = o;

  *ptr = n;
  MemoryBarrier();
}
