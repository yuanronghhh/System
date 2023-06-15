#include <System/Platform/Common/SysThreadPrivate.h>

/**
 * this code from glib GThread
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

typedef struct _SysUnixMutex SysUnixMutex;
typedef struct _SysThreadPosix SysThreadPosix;

/* Thread */
#define posix_check_err(err, name) { \
  int error = (err);        \
  if (error)           \
    sys_error ("file %s: line %d (%s): error '%s' during '%s'",  \
           __FILE__, __LINE__, __func__,  \
           sys_strerr (error), name);     \
  }

#define posix_check_cmd(cmd) posix_check_err (cmd, #cmd)

struct _SysThreadPosix {
  SysRealThread thread;

  pthread_t real_thread;
  SysBool  joined;
  SysMutex    lock;
};

void sys_real_thread_free (SysRealThread *thread) {
  SysThreadPosix *pt = (SysThreadPosix *) thread;

  if (!pt->joined) {

    pthread_detach (pt->real_thread);
  }

  sys_mutex_clear (&pt->lock);

  sys_slice_free (SysThreadPosix, pt);
}

SysRealThread * sys_real_thread_new (SysThreadFunc thread_func, SysError **error) {
  SysThreadPosix *thread;
  pthread_attr_t attr;
  SysInt ret;

  thread = sys_slice_new0 (SysThreadPosix);

  posix_check_cmd (pthread_attr_init (&attr));

  ret = pthread_create (&thread->real_thread, &attr, (void* (*)(void*))thread_func, thread);

  posix_check_cmd (pthread_attr_destroy (&attr));

  if (ret == EAGAIN) {
    sys_error_set_N (error, "Error creating thread: %s", sys_strerr (ret));
    sys_slice_free (SysThreadPosix, thread);
    return NULL;
  }

  posix_check_err (ret, "pthread_create");

  sys_mutex_init (&thread->lock);

  return (SysRealThread *) thread;
}

void sys_thread_yield (void) {
  sched_yield ();
}

void sys_real_thread_wait (SysRealThread *thread) {
  SysThreadPosix *pt = (SysThreadPosix *) thread;

  sys_mutex_lock (&pt->lock);

  if (!pt->joined) {
    posix_check_cmd (pthread_join (pt->real_thread, NULL));
    pt->joined = true;
  }

  sys_mutex_unlock (&pt->lock);
}

void sys_real_thread_exit (void) {
  pthread_exit (NULL);
}

void sys_real_thread_set_name (const SysChar *name) {
#if defined(HAVE_PTHREAD_SETNAME_NP_WITH_TID)
  pthread_setname_np (pthread_self(), name); /* on Linux and Solaris */
#elif defined(HAVE_PTHREAD_SETNAME_NP_WITHOUT_TID)
  pthread_setname_np (name); /* on OS X and iOS */
#endif
}

/* Mutex */
static pthread_mutex_t *sys_mutex_impl_new (void) {
  pthread_mutexattr_t *pattr = NULL;
  pthread_mutex_t *mutex;
  SysInt status;
  pthread_mutexattr_t attr;

  mutex = malloc (sizeof (pthread_mutex_t));
  if (mutex == NULL)
    sys_abort_N("thread abort: %d", errno);

  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
  pattr = &attr;

  if ((status = pthread_mutex_init (mutex, pattr)) != 0)
    sys_abort_N("thread abort: %d", status);

  pthread_mutexattr_destroy (&attr);

  return mutex;
}

static void sys_mutex_impl_free (pthread_mutex_t *mutex) {
  pthread_mutex_destroy (mutex);
  free (mutex);
}

static inline pthread_mutex_t *sys_mutex_get_impl (SysMutex *mutex) {
  pthread_mutex_t *impl = sys_atomic_pointer_get (&mutex->p);

  if (impl == NULL) {
    impl = sys_mutex_impl_new ();
  if (!sys_atomic_pointer_cmpxchg (&mutex->p, NULL, impl))
      sys_mutex_impl_free (impl);
    impl = mutex->p;
  }

  return impl;
}

void sys_mutex_init (SysMutex *mutex) {
  mutex->p = sys_mutex_impl_new ();
}

void sys_mutex_clear (SysMutex *mutex) {
  sys_mutex_impl_free (mutex->p);
}

void sys_mutex_lock (SysMutex *mutex) {
  SysInt status;

  if ((status = pthread_mutex_lock (sys_mutex_get_impl (mutex))) != 0)
    sys_abort_N("thread abort: %d", status);
}

void sys_mutex_unlock (SysMutex *mutex) {
  SysInt status;

  if ((status = pthread_mutex_unlock (sys_mutex_get_impl (mutex))) != 0)
    sys_abort_N("thread abort: %d", status);
}

SysBool sys_mutex_trylock (SysMutex *mutex) {
  SysInt status;

  if ((status = pthread_mutex_trylock (sys_mutex_get_impl (mutex))) == 0)
    return true;

  if (status != EBUSY)
    sys_abort_N("thread abort: %d", status);

  return false;
}

/* {{{1 SysRecMutex */

static pthread_mutex_t *sys_rec_mutex_impl_new (void) {
  pthread_mutexattr_t attr;
  pthread_mutex_t *mutex;

  mutex = malloc (sizeof (pthread_mutex_t));
  if (mutex == NULL)
    sys_abort_N("thread abort: %d", errno);

  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init (mutex, &attr);
  pthread_mutexattr_destroy (&attr);

  return mutex;
}

static void sys_rec_mutex_impl_free (pthread_mutex_t *mutex) {
  pthread_mutex_destroy (mutex);
  free (mutex);
}

static inline pthread_mutex_t *sys_rec_mutex_get_impl (SysRecMutex *rec_mutex) {
  pthread_mutex_t *impl = sys_atomic_pointer_get (&rec_mutex->p);

  if (impl == NULL)
  {
    impl = sys_rec_mutex_impl_new ();
    if (!sys_atomic_pointer_cmpxchg (&rec_mutex->p, NULL, impl))
      sys_rec_mutex_impl_free (impl);
    impl = rec_mutex->p;
  }

  return impl;
}

void sys_rec_mutex_init (SysRecMutex *rec_mutex) {
  rec_mutex->p = sys_rec_mutex_impl_new ();
}

void sys_rec_mutex_clear (SysRecMutex *rec_mutex) {
  sys_rec_mutex_impl_free (rec_mutex->p);
}

void sys_rec_mutex_lock (SysRecMutex *mutex) {
  pthread_mutex_lock (sys_rec_mutex_get_impl (mutex));
}

void sys_rec_mutex_unlock (SysRecMutex *rec_mutex) {
  pthread_mutex_unlock (rec_mutex->p);
}

SysBool sys_rec_mutex_trylock (SysRecMutex *rec_mutex) {
  if (pthread_mutex_trylock (sys_rec_mutex_get_impl (rec_mutex)) != 0)
    return false;

  return true;
}

/* {{{1 SysRWLock */

static pthread_rwlock_t *sys_rw_lock_impl_new (void) {
  pthread_rwlock_t *rwlock;
  SysInt status;

  rwlock = malloc (sizeof (pthread_rwlock_t));
  if (rwlock == NULL)
    sys_abort_N("thread abort: %d", errno);

  if ((status = pthread_rwlock_init (rwlock, NULL)) != 0)
    sys_abort_N("thread abort: %d", status);

  return rwlock;
}

static void sys_rw_lock_impl_free (pthread_rwlock_t *rwlock) {
  pthread_rwlock_destroy (rwlock);
  free (rwlock);
}

static inline pthread_rwlock_t *sys_rw_lock_get_impl (SysRWLock *lock) {
  pthread_rwlock_t *impl = sys_atomic_pointer_get (&lock->p);

  if (impl == NULL)
  {
    impl = sys_rw_lock_impl_new ();
    if (!sys_atomic_pointer_cmpxchg (&lock->p, NULL, impl))
      sys_rw_lock_impl_free (impl);
    impl = lock->p;
  }

  return impl;
}

void sys_rw_lock_init (SysRWLock *rw_lock) {
  rw_lock->p = sys_rw_lock_impl_new ();
}

void sys_rw_lock_clear (SysRWLock *rw_lock) {
  sys_rw_lock_impl_free (rw_lock->p);
}

void sys_rw_lock_writer_lock (SysRWLock *rw_lock) {
  int retval = pthread_rwlock_wrlock (sys_rw_lock_get_impl (rw_lock));

  if (retval != 0)
    sys_abort_N ("Failed to get RW lock %p: %s", rw_lock, sys_strerr (retval));
}

SysBool sys_rw_lock_writer_trylock (SysRWLock *rw_lock) {
  if (pthread_rwlock_trywrlock (sys_rw_lock_get_impl (rw_lock)) != 0)
    return false;

  return true;
}

void sys_rw_lock_writer_unlock (SysRWLock *rw_lock) {
  pthread_rwlock_unlock (sys_rw_lock_get_impl (rw_lock));
}

void sys_rw_lock_reader_lock (SysRWLock *rw_lock) {
  int retval = pthread_rwlock_rdlock (sys_rw_lock_get_impl (rw_lock));

  if (retval != 0)
    sys_abort_N ("Failed to get RW lock %p: %s", rw_lock, sys_strerr (retval));
}

SysBool sys_rw_lock_reader_trylock (SysRWLock *rw_lock) {
  if (pthread_rwlock_tryrdlock (sys_rw_lock_get_impl (rw_lock)) != 0)
    return false;

  return true;
}

void sys_rw_lock_reader_unlock (SysRWLock *rw_lock) {
  pthread_rwlock_unlock (sys_rw_lock_get_impl (rw_lock));
}

/* {{{1 SysCond */

static pthread_cond_t *sys_cond_impl_new (void) {
  pthread_condattr_t attr;
  pthread_cond_t *cond;
  SysInt status;

  pthread_condattr_init (&attr);

  if ((status = pthread_condattr_setclock (&attr, CLOCK_MONOTONIC)) != 0) {
    sys_abort_N("thread abort: %d", status);
  }

  cond = malloc (sizeof (pthread_cond_t));
  if (cond == NULL)
    sys_abort_N("thread abort: %d", errno);

  if ((status = pthread_cond_init (cond, &attr)) != 0)
    sys_abort_N("thread abort: %d", status);

  pthread_condattr_destroy (&attr);

  return cond;
}

static void sys_cond_impl_free (pthread_cond_t *cond) {
  pthread_cond_destroy (cond);
  free (cond);
}

static inline pthread_cond_t *sys_cond_get_impl (SysCond *cond) {
  pthread_cond_t *impl = sys_atomic_pointer_get (&cond->p);

  if (impl == NULL)
  {
    impl = sys_cond_impl_new ();
    if (!sys_atomic_pointer_cmpxchg (&cond->p, NULL, impl))
      sys_cond_impl_free (impl);
    impl = cond->p;
  }

  return impl;
}

void sys_cond_init (SysCond *cond) {
  cond->p = sys_cond_impl_new ();
}

void sys_cond_clear (SysCond *cond) {
  sys_cond_impl_free (cond->p);
}

void sys_cond_wait (SysCond  *cond, SysMutex *mutex) {
  SysInt status;

  if ((status = pthread_cond_wait (sys_cond_get_impl (cond), sys_mutex_get_impl (mutex))) != 0)
    sys_abort_N("thread abort: %d", status);
}

void sys_cond_signal (SysCond *cond) {
  SysInt status;

  if ((status = pthread_cond_signal (sys_cond_get_impl (cond))) != 0)
    sys_abort_N("thread abort: %d", status);
}

void sys_cond_broadcast (SysCond *cond) {
  SysInt status;

  if ((status = pthread_cond_broadcast (sys_cond_get_impl (cond))) != 0)
    sys_abort_N("thread abort: %d", status);
}

SysBool sys_cond_wait_until (SysCond  *cond, SysMutex *mutex, SysInt64  end_time) {
  struct timespec ts;
  SysInt status;

  ts.tv_sec = end_time / 1000000;
  ts.tv_nsec = (end_time % 1000000) * 1000;

  if ((status = pthread_cond_timedwait (sys_cond_get_impl (cond), sys_mutex_get_impl (mutex), &ts)) == 0)
    return true;

  if (status != ETIMEDOUT)
    sys_abort_N("thread abort: %d", status);

  return false;
}
