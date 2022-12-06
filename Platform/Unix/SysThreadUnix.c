#include <Platform/SysThreadPrivate.h>

typedef struct _SysUnixThread SysUnixThread;
typedef struct _SysUnixMutex SysUnixMutex;

struct _SysUnixThread {
  SysRealThread parent;
  SysThreadFunc proxy;
  pthread_t     *thread;
};

struct _SysUnixMutex {
  SysRealMutex parent;
};

SysPointer sys_real_private_get(SysPrivate *key) {
  sys_return_val_if_fail(key == NULL, NULL);

  pthread_key_t *pkey = (pthread_key_t *)key->p;

  return pthread_getspecific (*pkey);
}

void sys_real_private_set (SysPrivate *key, SysPointer value) {
  sys_return_if_fail(key == NULL);
  sys_return_if_fail(value == NULL);

  SysInt status;
  pthread_key_t *pkey = (pthread_key_t *)key->p;

  status = pthread_setspecific (*pkey, value);
  sys_abort_E(status != 0, "unix private_set pthread_setspecific failed.");
}

void sys_real_thread_init(void) {
}

void sys_real_thread_detach(void) {
}

SysRealThread *sys_real_thread_new(SysThreadFunc proxy, SysError **error) {
  sys_return_val_if_fail(proxy == NULL, NULL);
  sys_return_val_if_fail(*error != NULL, NULL);

  return NULL;
}

void sys_real_thread_exit(void) {
}

void sys_real_thread_wait(SysRealThread *thread) {
  sys_return_if_fail(thread == NULL);

}

void sys_real_thread_free(SysRealThread *thread) {
  sys_return_if_fail(thread == NULL);

}

void sys_real_mutex_free(SysRealMutex *mutex) {
  sys_return_if_fail(mutex == NULL);
}

bool sys_real_mutex_trylock(SysRealMutex *mutex) {
  sys_return_val_if_fail(mutex == NULL, false);

  return true;
}

SysRealMutex *sys_real_mutex_new(void) {
  pthread_mutexattr_t attr;
  pthread_mutex_t *lock;
  SysInt status;
  SysMutex *base;
  SysUnixMutex *posix; /* NOTE: use unix name will occur error. */

  posix = sys_new_N(SysUnixMutex, 1);
  base = (SysMutex *)posix;

  lock = sys_new_N(pthread_mutex_t, 1);
  sys_abort_E(lock != NULL, "sys_real_mutex_new pthread_mutex_t create malloc failed.");

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);

  status = pthread_mutex_init(lock, &attr);
  sys_abort_E(status == 0, "pthread_mutex_init should not be failed.");
  pthread_mutexattr_destroy(&attr);

  base->lock = lock;
  return (SysRealMutex *)posix;
}

void sys_real_mutex_lock(SysRealMutex *mutex) {
  sys_return_if_fail(mutex == NULL);

  SysMutex *base = (SysMutex *)mutex;
  int status;

  status = pthread_mutex_lock(base->lock);
  sys_abort_E(status == 0, "pthread_mutex_lock lock failed.");
}

void sys_real_mutex_unlock(SysRealMutex *mutex) {
  sys_return_if_fail(mutex == NULL);

  SysMutex *base = (SysMutex *)mutex;
  int status;

  status = pthread_mutex_unlock(base->lock);
  sys_abort_E(status == 0, "pthread_mutex_unlock unlock failed.");
}
