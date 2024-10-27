#include <System/Platform/Common/SysAtomic.h>

SysInt _sys_atomic_int_get(const volatile SysInt *i) {
  SysInt gaig_temp;
  __atomic_load ((SysInt *)(i), &gaig_temp, __ATOMIC_SEQ_CST);
  return (SysInt)gaig_temp;
}

void _sys_atomic_int_set(volatile SysInt *x, SysInt n) {
  SysInt gais_temp = (SysInt) (n);
  __atomic_store ((SysInt *)(x), &gais_temp, __ATOMIC_SEQ_CST);
}

void sys_atomic_int_inc(SysInt *x) {
  __atomic_fetch_add ((x), 1, __ATOMIC_SEQ_CST);
}

SysBool sys_atomic_int_dec_and_test(SysInt *x) {
  return __atomic_fetch_sub ((x), 1, __ATOMIC_SEQ_CST) == 1;
}

SysBool sys_atomic_int_dec(SysInt *x) {
  return sys_atomic_int_dec_and_test(x);
}

SysBool sys_atomic_int_cmpxchg(SysInt *x, SysInt o, SysInt n) {
  SysInt gaicae_oldval = (o);

  return __atomic_compare_exchange_n ((x), (void *) (&(gaicae_oldval)), (n), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? true : false;
}

SysBool sys_atomic_pointer_cmpxchg(volatile SysPointer* x, SysPointer o, SysPointer n) {
  SysPointer gaicae_oldval = (o);

  return __atomic_compare_exchange_n ((x), (void *) (&(gaicae_oldval)), (n), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? true : false;
}

SysPointer sys_atomic_pointer_get(SysPointer x) {
  SysPointer n;
  SysPointer *o = (SysPointer *)x;

  __atomic_load(o, &n, __ATOMIC_SEQ_CST);
  return n;
}

void sys_atomic_pointer_set(volatile SysPointer o, SysPointer n) {
  SysPointer *no = (SysPointer *)(o);
  SysPointer nn = (SysPointer)(n);

  __atomic_store (no, &nn, __ATOMIC_SEQ_CST);
}
