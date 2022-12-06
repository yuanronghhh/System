#include <Platform/SysThreadPrivate.h>

static void sys_thread_clean(SysPointer data);

static SysPrivate private_thread = { NULL, sys_thread_clean };

void sys_private_set(SysPrivate *key, SysPointer value) {
  sys_real_private_set(key, value);
}

SysPointer sys_private_get(SysPrivate *key) {
  return sys_real_private_get(key);
}

static void sys_thread_clean(SysPointer data) {
  sys_thread_unref(data);
}

static SysPointer sys_thread_proxy(SysPointer data) {
  SysRealThread* thread = data;

  sys_private_set(&private_thread, data);

  thread->retval = thread->thread.func(thread->thread.data);

  return thread->retval;
}

void sys_thread_init(void) {
  sys_real_thread_init();
}

void sys_thread_detach(void) {
  sys_real_thread_detach();
}

void sys_thread_exit(void) {
  sys_real_thread_exit();
}

SysThread *sys_thread_new(SysThreadFunc func, SysPointer data) {
  SysError *err = NULL;

  SysRealThread *real = sys_real_thread_new(sys_thread_proxy, &err);
  sys_return_val_if_fail(real != NULL, NULL);

  real->thread.func = func;
  real->thread.data = data;

  return (SysThread *)real;
}

SysPointer sys_thread_join(SysThread *thread) {
  SysRealThread *real = (SysRealThread*)thread;
  SysPointer retval;

  sys_return_val_if_fail(thread, NULL);

  sys_real_thread_wait(real);

  retval = real->retval;

  sys_thread_unref(thread);

  return retval;
}

SysThread* sys_thread_self(void) {
  SysRealThread* thread = sys_private_get(&private_thread);

  sys_abort_E(thread != NULL, "thread may not created by sys_thread_new.");

  sys_private_set(&private_thread, thread);

  return (SysThread*)thread;
}

void sys_thread_unref(SysThread *thread) {
  SysRealThread *real = (SysRealThread *)thread;

  if (!sys_ref_count_dec(real)) {
    return;
  }
  
  sys_real_thread_free(real);
}

SysMutex *sys_mutex_new(void) {
  SysRealMutex *real = sys_real_mutex_new();

  return (SysMutex *)real;
}

void sys_mutex_free(SysMutex *mutex) {
  SysRealMutex *real = (SysRealMutex *)mutex;

  sys_real_mutex_free(real);
}

void sys_mutex_lock(SysMutex *mutex) {
  SysRealMutex *real = (SysRealMutex *)mutex;

  sys_real_mutex_lock(real);
}

bool sys_mutex_trylock(SysMutex *mutex) {
  SysRealMutex *real = (SysRealMutex *)mutex;

  return sys_real_mutex_trylock(real);
}

void sys_mutex_unlock(SysMutex *mutex) {
  SysRealMutex *real = (SysRealMutex *)mutex;

  sys_real_mutex_unlock(real);
}
