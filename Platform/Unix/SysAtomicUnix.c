#include <System/Platform/Common/SysAtomicPrivate.h>

SysInt sys_real_atomic_int_get(const volatile SysInt *i) {
  __sync_synchronize();
  return (SysInt)*(i);
}

void sys_real_atomic_int_set(volatile SysInt *x, SysInt n) {
  *(x) = (n);
  __sync_synchronize ();
}

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

bool sys_real_atomic_pointer_cmpxchg(SysPointer *x, SysPointer o, SysPointer n) {
  return __sync_bool_compare_and_swap (x, o, n);
}

SysPointer sys_real_atomic_pointer_get(SysPointer x) {
  const SysPointer *ptr = x;
  __sync_synchronize ();
  __asm__ __volatile__ ("" : : : "memory");

  return *(ptr);
}

void sys_real_atomic_pointer_set(volatile SysPointer o, SysPointer n) {
  volatile SysPointer *ptr = o;

  *ptr = n;
  __sync_synchronize ();
}
