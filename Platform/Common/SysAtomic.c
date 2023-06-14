#include <System/Platform/Common/SysAtomicPrivate.h>

void sys_ref_count_log(const char *func_name, SysInt i) {
  fprintf(stdout, "[%s] %d\n", func_name, i);
}

void sys_atomic_int_inc(SysInt *x) {
  sys_real_atomic_int_inc(x);
}

SysInt _sys_atomic_int_get(const volatile SysInt *x) {
  return sys_real_atomic_int_get(x);
}

void _sys_atomic_int_set(volatile SysInt *x, SysInt n) {
  sys_real_atomic_int_set(x, n);
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

SysPointer sys_atomic_ptr_get(const volatile SysPointer x) {
  return sys_real_atomic_ptr_get(x);
}

void sys_atomic_ptr_set(void* o, SysPointer n) {
  sys_real_atomic_ptr_set(o, n);
}
