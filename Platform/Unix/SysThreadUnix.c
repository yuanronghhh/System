#include <System/Utils/SysStr.h>
#include <System/Platform/Common/SysThreadPrivate.h>

/**
 * this code from glib SysThread
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

void sys_system_thread_init(void) {
}

void sys_system_thread_detach(void) {
}

static void
sys_thread_abort (SysInt         status,
                const SysChar *function)
{
  sys_abort_N ("GLib (gthread-posix.c): Unexpected error from C library during '%s': %s.  Aborting.\n",
           function, strerror (status));
}

/* {{{1 SysMutex */

#if !defined(USE_NATIVE_MUTEX)

static pthread_mutex_t *
sys_mutex_impl_new (void)
{
  pthread_mutexattr_t *pattr = NULL;
  pthread_mutex_t *mutex;
  SysInt status;
#ifdef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
  pthread_mutexattr_t attr;
#endif

  mutex = malloc (sizeof (pthread_mutex_t));
  if SYS_UNLIKELY (mutex == NULL)
    sys_thread_abort (errno, "malloc");

#ifdef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
  pattr = &attr;
#endif

  if SYS_UNLIKELY ((status = pthread_mutex_init (mutex, pattr)) != 0)
    sys_thread_abort (status, "pthread_mutex_init");

#ifdef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
  pthread_mutexattr_destroy (&attr);
#endif

  return mutex;
}

static void
sys_mutex_impl_free (pthread_mutex_t *mutex)
{
  pthread_mutex_destroy (mutex);
  free (mutex);
}

static inline pthread_mutex_t *
sys_mutex_get_impl (SysMutex *mutex)
{
  pthread_mutex_t *impl = sys_atomic_pointer_get (&mutex->p);

  if SYS_UNLIKELY (impl == NULL)
    {
      impl = sys_mutex_impl_new ();
      if (!sys_atomic_pointer_cmpxchg (&mutex->p, NULL, impl))
        sys_mutex_impl_free (impl);
      impl = mutex->p;
    }

  return impl;
}


/**
 * sys_mutex_init:
 * @mutex: an uninitialized #SysMutex
 *
 * Initializes a #SysMutex so that it can be used.
 *
 * This function is useful to initialize a mutex that has been
 * allocated on the stack, or as part of a larger structure.
 * It is not necessary to initialize a mutex that has been
 * statically allocated.
 *
 * |[<!-- language="C" --> 
 *   typedef struct {
 *     SysMutex m;
 *     ...
 *   } Blob;
 *
 * Blob *b;
 *
 * b = sys_new (Blob, 1);
 * sys_mutex_init (&b->m);
 * ]|
 *
 * To undo the effect of sys_mutex_init() when a mutex is no longer
 * needed, use sys_mutex_clear().
 *
 * Calling sys_mutex_init() on an already initialized #SysMutex leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_mutex_init (SysMutex *mutex)
{
  mutex->p = sys_mutex_impl_new ();
}

/**
 * sys_mutex_clear:
 * @mutex: an initialized #SysMutex
 *
 * Frees the resources allocated to a mutex with sys_mutex_init().
 *
 * This function should not be used with a #SysMutex that has been
 * statically allocated.
 *
 * Calling sys_mutex_clear() on a locked mutex leads to undefined
 * behaviour.
 *
 * Since: 2.32
 */
void
sys_mutex_clear (SysMutex *mutex)
{
  sys_mutex_impl_free (mutex->p);
}

/**
 * sys_mutex_lock:
 * @mutex: a #SysMutex
 *
 * Locks @mutex. If @mutex is already locked by another thread, the
 * current thread will block until @mutex is unlocked by the other
 * thread.
 *
 * #SysMutex is neither guaranteed to be recursive nor to be
 * non-recursive.  As such, calling sys_mutex_lock() on a #SysMutex that has
 * already been locked by the same thread results in undefined behaviour
 * (including but not limited to deadlocks).
 */
void
sys_mutex_lock (SysMutex *mutex)
{
  SysInt status;

  if SYS_UNLIKELY ((status = pthread_mutex_lock (sys_mutex_get_impl (mutex))) != 0)
    sys_thread_abort (status, "pthread_mutex_lock");
}

/**
 * sys_mutex_unlock:
 * @mutex: a #SysMutex
 *
 * Unlocks @mutex. If another thread is blocked in a sys_mutex_lock()
 * call for @mutex, it will become unblocked and can lock @mutex itself.
 *
 * Calling sys_mutex_unlock() on a mutex that is not locked by the
 * current thread leads to undefined behaviour.
 */
void
sys_mutex_unlock (SysMutex *mutex)
{
  SysInt status;

  if SYS_UNLIKELY ((status = pthread_mutex_unlock (sys_mutex_get_impl (mutex))) != 0)
    sys_thread_abort (status, "pthread_mutex_unlock");
}

/**
 * sys_mutex_trylock:
 * @mutex: a #SysMutex
 *
 * Tries to lock @mutex. If @mutex is already locked by another thread,
 * it immediately returns %false. Otherwise it locks @mutex and returns
 * %true.
 *
 * #SysMutex is neither guaranteed to be recursive nor to be
 * non-recursive.  As such, calling sys_mutex_lock() on a #SysMutex that has
 * already been locked by the same thread results in undefined behaviour
 * (including but not limited to deadlocks or arbitrary return values).
 *
 * Returns: %true if @mutex could be locked
 */
SysBool
sys_mutex_trylock (SysMutex *mutex)
{
  SysInt status;

  if SYS_LIKELY ((status = pthread_mutex_trylock (sys_mutex_get_impl (mutex))) == 0)
    return true;

  if SYS_UNLIKELY (status != EBUSY)
    sys_thread_abort (status, "pthread_mutex_trylock");

  return false;
}

#endif /* !defined(USE_NATIVE_MUTEX) */

/* {{{1 SysRecMutex */

static pthread_mutex_t *
sys_rec_mutex_impl_new (void)
{
  pthread_mutexattr_t attr;
  pthread_mutex_t *mutex;

  mutex = malloc (sizeof (pthread_mutex_t));
  if SYS_UNLIKELY (mutex == NULL)
    sys_thread_abort (errno, "malloc");

  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init (mutex, &attr);
  pthread_mutexattr_destroy (&attr);

  return mutex;
}

static void
sys_rec_mutex_impl_free (pthread_mutex_t *mutex)
{
  pthread_mutex_destroy (mutex);
  free (mutex);
}

static inline pthread_mutex_t *
sys_rec_mutex_get_impl (SysRecMutex *rec_mutex)
{
  pthread_mutex_t *impl = sys_atomic_pointer_get (&rec_mutex->p);

  if SYS_UNLIKELY (impl == NULL)
    {
      impl = sys_rec_mutex_impl_new ();
      if (!sys_atomic_pointer_cmpxchg (&rec_mutex->p, NULL, impl))
        sys_rec_mutex_impl_free (impl);
      impl = rec_mutex->p;
    }

  return impl;
}

/**
 * sys_rec_mutex_init:
 * @rec_mutex: an uninitialized #SysRecMutex
 *
 * Initializes a #SysRecMutex so that it can be used.
 *
 * This function is useful to initialize a recursive mutex
 * that has been allocated on the stack, or as part of a larger
 * structure.
 *
 * It is not necessary to initialise a recursive mutex that has been
 * statically allocated.
 *
 * |[<!-- language="C" --> 
 *   typedef struct {
 *     SysRecMutex m;
 *     ...
 *   } Blob;
 *
 * Blob *b;
 *
 * b = sys_new (Blob, 1);
 * sys_rec_mutex_init (&b->m);
 * ]|
 *
 * Calling sys_rec_mutex_init() on an already initialized #SysRecMutex
 * leads to undefined behaviour.
 *
 * To undo the effect of sys_rec_mutex_init() when a recursive mutex
 * is no longer needed, use sys_rec_mutex_clear().
 *
 * Since: 2.32
 */
void
sys_rec_mutex_init (SysRecMutex *rec_mutex)
{
  rec_mutex->p = sys_rec_mutex_impl_new ();
}

/**
 * sys_rec_mutex_clear:
 * @rec_mutex: an initialized #SysRecMutex
 *
 * Frees the resources allocated to a recursive mutex with
 * sys_rec_mutex_init().
 *
 * This function should not be used with a #SysRecMutex that has been
 * statically allocated.
 *
 * Calling sys_rec_mutex_clear() on a locked recursive mutex leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rec_mutex_clear (SysRecMutex *rec_mutex)
{
  sys_rec_mutex_impl_free (rec_mutex->p);
}

/**
 * sys_rec_mutex_lock:
 * @rec_mutex: a #SysRecMutex
 *
 * Locks @rec_mutex. If @rec_mutex is already locked by another
 * thread, the current thread will block until @rec_mutex is
 * unlocked by the other thread. If @rec_mutex is already locked
 * by the current thread, the 'lock count' of @rec_mutex is increased.
 * The mutex will only become available again when it is unlocked
 * as many times as it has been locked.
 *
 * Since: 2.32
 */
void
sys_rec_mutex_lock (SysRecMutex *mutex)
{
  pthread_mutex_lock (sys_rec_mutex_get_impl (mutex));
}

/**
 * sys_rec_mutex_unlock:
 * @rec_mutex: a #SysRecMutex
 *
 * Unlocks @rec_mutex. If another thread is blocked in a
 * sys_rec_mutex_lock() call for @rec_mutex, it will become unblocked
 * and can lock @rec_mutex itself.
 *
 * Calling sys_rec_mutex_unlock() on a recursive mutex that is not
 * locked by the current thread leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rec_mutex_unlock (SysRecMutex *rec_mutex)
{
  pthread_mutex_unlock (rec_mutex->p);
}

/**
 * sys_rec_mutex_trylock:
 * @rec_mutex: a #SysRecMutex
 *
 * Tries to lock @rec_mutex. If @rec_mutex is already locked
 * by another thread, it immediately returns %false. Otherwise
 * it locks @rec_mutex and returns %true.
 *
 * Returns: %true if @rec_mutex could be locked
 *
 * Since: 2.32
 */
SysBool
sys_rec_mutex_trylock (SysRecMutex *rec_mutex)
{
  if (pthread_mutex_trylock (sys_rec_mutex_get_impl (rec_mutex)) != 0)
    return false;

  return true;
}

/* {{{1 SysRWLock */

static pthread_rwlock_t *
sys_rw_lock_impl_new (void)
{
  pthread_rwlock_t *rwlock;
  SysInt status;

  rwlock = malloc (sizeof (pthread_rwlock_t));
  if SYS_UNLIKELY (rwlock == NULL)
    sys_thread_abort (errno, "malloc");

  if SYS_UNLIKELY ((status = pthread_rwlock_init (rwlock, NULL)) != 0)
    sys_thread_abort (status, "pthread_rwlock_init");

  return rwlock;
}

static void
sys_rw_lock_impl_free (pthread_rwlock_t *rwlock)
{
  pthread_rwlock_destroy (rwlock);
  free (rwlock);
}

static inline pthread_rwlock_t *
sys_rw_lock_get_impl (SysRWLock *lock)
{
  pthread_rwlock_t *impl = sys_atomic_pointer_get (&lock->p);

  if SYS_UNLIKELY (impl == NULL)
    {
      impl = sys_rw_lock_impl_new ();
      if (!sys_atomic_pointer_cmpxchg (&lock->p, NULL, impl))
        sys_rw_lock_impl_free (impl);
      impl = lock->p;
    }

  return impl;
}

/**
 * sys_rw_lock_init:
 * @rw_lock: an uninitialized #SysRWLock
 *
 * Initializes a #SysRWLock so that it can be used.
 *
 * This function is useful to initialize a lock that has been
 * allocated on the stack, or as part of a larger structure.  It is not
 * necessary to initialise a reader-writer lock that has been statically
 * allocated.
 *
 * |[<!-- language="C" --> 
 *   typedef struct {
 *     SysRWLock l;
 *     ...
 *   } Blob;
 *
 * Blob *b;
 *
 * b = sys_new (Blob, 1);
 * sys_rw_lock_init (&b->l);
 * ]|
 *
 * To undo the effect of sys_rw_lock_init() when a lock is no longer
 * needed, use sys_rw_lock_clear().
 *
 * Calling sys_rw_lock_init() on an already initialized #SysRWLock leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rw_lock_init (SysRWLock *rw_lock)
{
  rw_lock->p = sys_rw_lock_impl_new ();
}

/**
 * sys_rw_lock_clear:
 * @rw_lock: an initialized #SysRWLock
 *
 * Frees the resources allocated to a lock with sys_rw_lock_init().
 *
 * This function should not be used with a #SysRWLock that has been
 * statically allocated.
 *
 * Calling sys_rw_lock_clear() when any thread holds the lock
 * leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rw_lock_clear (SysRWLock *rw_lock)
{
  sys_rw_lock_impl_free (rw_lock->p);
}

/**
 * sys_rw_lock_writer_lock:
 * @rw_lock: a #SysRWLock
 *
 * Obtain a write lock on @rw_lock. If another thread currently holds
 * a read or write lock on @rw_lock, the current thread will block
 * until all other threads have dropped their locks on @rw_lock.
 *
 * Calling sys_rw_lock_writer_lock() while the current thread already
 * owns a read or write lock on @rw_lock leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rw_lock_writer_lock (SysRWLock *rw_lock)
{
  SysInt retval = pthread_rwlock_wrlock (sys_rw_lock_get_impl (rw_lock));

  if (retval != 0)
    sys_abort_N ("Failed to get RW lock %p: %s", rw_lock, sys_strerror (retval));
}

/**
 * sys_rw_lock_writer_trylock:
 * @rw_lock: a #SysRWLock
 *
 * Tries to obtain a write lock on @rw_lock. If another thread
 * currently holds a read or write lock on @rw_lock, it immediately
 * returns %false.
 * Otherwise it locks @rw_lock and returns %true.
 *
 * Returns: %true if @rw_lock could be locked
 *
 * Since: 2.32
 */
SysBool
sys_rw_lock_writer_trylock (SysRWLock *rw_lock)
{
  if (pthread_rwlock_trywrlock (sys_rw_lock_get_impl (rw_lock)) != 0)
    return false;

  return true;
}

/**
 * sys_rw_lock_writer_unlock:
 * @rw_lock: a #SysRWLock
 *
 * Release a write lock on @rw_lock.
 *
 * Calling sys_rw_lock_writer_unlock() on a lock that is not held
 * by the current thread leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rw_lock_writer_unlock (SysRWLock *rw_lock)
{
  pthread_rwlock_unlock (sys_rw_lock_get_impl (rw_lock));
}

/**
 * sys_rw_lock_reader_lock:
 * @rw_lock: a #SysRWLock
 *
 * Obtain a read lock on @rw_lock. If another thread currently holds
 * the write lock on @rw_lock, the current thread will block until the
 * write lock was (held and) released. If another thread does not hold
 * the write lock, but is waiting for it, it is implementation defined
 * whether the reader or writer will block. Read locks can be taken
 * recursively.
 *
 * Calling sys_rw_lock_reader_lock() while the current thread already
 * owns a write lock leads to undefined behaviour. Read locks however
 * can be taken recursively, in which case you need to make sure to
 * call sys_rw_lock_reader_unlock() the same amount of times.
 *
 * It is implementation-defined how many read locks are allowed to be
 * held on the same lock simultaneously. If the limit is hit,
 * or if a deadlock is detected, a critical warning will be emitted.
 *
 * Since: 2.32
 */
void
sys_rw_lock_reader_lock (SysRWLock *rw_lock)
{
  SysInt retval = pthread_rwlock_rdlock (sys_rw_lock_get_impl (rw_lock));

  if (retval != 0)
    sys_abort_N ("Failed to get RW lock %p: %s", rw_lock, sys_strerror (retval));
}

/**
 * sys_rw_lock_reader_trylock:
 * @rw_lock: a #SysRWLock
 *
 * Tries to obtain a read lock on @rw_lock and returns %true if
 * the read lock was successfully obtained. Otherwise it
 * returns %false.
 *
 * Returns: %true if @rw_lock could be locked
 *
 * Since: 2.32
 */
SysBool
sys_rw_lock_reader_trylock (SysRWLock *rw_lock)
{
  if (pthread_rwlock_tryrdlock (sys_rw_lock_get_impl (rw_lock)) != 0)
    return false;

  return true;
}

/**
 * sys_rw_lock_reader_unlock:
 * @rw_lock: a #SysRWLock
 *
 * Release a read lock on @rw_lock.
 *
 * Calling sys_rw_lock_reader_unlock() on a lock that is not held
 * by the current thread leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_rw_lock_reader_unlock (SysRWLock *rw_lock)
{
  pthread_rwlock_unlock (sys_rw_lock_get_impl (rw_lock));
}

/* {{{1 SysCond */

#if !defined(USE_NATIVE_MUTEX)

static pthread_cond_t *
sys_cond_impl_new (void)
{
  pthread_condattr_t attr;
  pthread_cond_t *cond;
  SysInt status;

  pthread_condattr_init (&attr);

  if SYS_UNLIKELY ((status = pthread_condattr_setclock (&attr, CLOCK_MONOTONIC)) != 0)
    sys_thread_abort (status, "pthread_condattr_setclock");

  cond = malloc (sizeof (pthread_cond_t));
  if SYS_UNLIKELY (cond == NULL)
    sys_thread_abort (errno, "malloc");

  if SYS_UNLIKELY ((status = pthread_cond_init (cond, &attr)) != 0)
    sys_thread_abort (status, "pthread_cond_init");

  pthread_condattr_destroy (&attr);

  return cond;
}

static void
sys_cond_impl_free (pthread_cond_t *cond)
{
  pthread_cond_destroy (cond);
  free (cond);
}

static inline pthread_cond_t *
sys_cond_get_impl (SysCond *cond)
{
  pthread_cond_t *impl = sys_atomic_pointer_get (&cond->p);

  if SYS_UNLIKELY (impl == NULL)
    {
      impl = sys_cond_impl_new ();
      if (!sys_atomic_pointer_cmpxchg (&cond->p, NULL, impl))
        sys_cond_impl_free (impl);
      impl = cond->p;
    }

  return impl;
}

/**
 * sys_cond_init:
 * @cond: an uninitialized #SysCond
 *
 * Initialises a #SysCond so that it can be used.
 *
 * This function is useful to initialise a #SysCond that has been
 * allocated as part of a larger structure.  It is not necessary to
 * initialise a #SysCond that has been statically allocated.
 *
 * To undo the effect of sys_cond_init() when a #SysCond is no longer
 * needed, use sys_cond_clear().
 *
 * Calling sys_cond_init() on an already-initialised #SysCond leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_cond_init (SysCond *cond)
{
  cond->p = sys_cond_impl_new ();
}

/**
 * sys_cond_clear:
 * @cond: an initialised #SysCond
 *
 * Frees the resources allocated to a #SysCond with sys_cond_init().
 *
 * This function should not be used with a #SysCond that has been
 * statically allocated.
 *
 * Calling sys_cond_clear() for a #SysCond on which threads are
 * blocking leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
sys_cond_clear (SysCond *cond)
{
  sys_cond_impl_free (cond->p);
}

/**
 * sys_cond_wait:
 * @cond: a #SysCond
 * @mutex: a #SysMutex that is currently locked
 *
 * Atomically releases @mutex and waits until @cond is signalled.
 * When this function returns, @mutex is locked again and owned by the
 * calling thread.
 *
 * When using condition variables, it is possible that a spurious wakeup
 * may occur (ie: sys_cond_wait() returns even though sys_cond_signal() was
 * not called).  It's also possible that a stolen wakeup may occur.
 * This is when sys_cond_signal() is called, but another thread acquires
 * @mutex before this thread and modifies the state of the program in
 * such a way that when sys_cond_wait() is able to return, the expected
 * condition is no longer met.
 *
 * For this reason, sys_cond_wait() must always be used in a loop.  See
 * the documentation for #SysCond for a complete example.
 **/
void
sys_cond_wait (SysCond  *cond,
             SysMutex *mutex)
{
  SysInt status;

  if SYS_UNLIKELY ((status = pthread_cond_wait (sys_cond_get_impl (cond), sys_mutex_get_impl (mutex))) != 0)
    sys_thread_abort (status, "pthread_cond_wait");
}

/**
 * sys_cond_signal:
 * @cond: a #SysCond
 *
 * If threads are waiting for @cond, at least one of them is unblocked.
 * If no threads are waiting for @cond, this function has no effect.
 * It is good practice to hold the same lock as the waiting thread
 * while calling this function, though not required.
 */
void
sys_cond_signal (SysCond *cond)
{
  SysInt status;

  if SYS_UNLIKELY ((status = pthread_cond_signal (sys_cond_get_impl (cond))) != 0)
    sys_thread_abort (status, "pthread_cond_signal");
}

/**
 * sys_cond_broadcast:
 * @cond: a #SysCond
 *
 * If threads are waiting for @cond, all of them are unblocked.
 * If no threads are waiting for @cond, this function has no effect.
 * It is good practice to lock the same mutex as the waiting threads
 * while calling this function, though not required.
 */
void
sys_cond_broadcast (SysCond *cond)
{
  SysInt status;

  if SYS_UNLIKELY ((status = pthread_cond_broadcast (sys_cond_get_impl (cond))) != 0)
    sys_thread_abort (status, "pthread_cond_broadcast");
}

/**
 * sys_cond_wait_until:
 * @cond: a #SysCond
 * @mutex: a #SysMutex that is currently locked
 * @end_time: the monotonic time to wait until
 *
 * Waits until either @cond is signalled or @end_time has passed.
 *
 * As with sys_cond_wait() it is possible that a spurious or stolen wakeup
 * could occur.  For that reason, waiting on a condition variable should
 * always be in a loop, based on an explicitly-checked predicate.
 *
 * %true is returned if the condition variable was signalled (or in the
 * case of a spurious wakeup).  %false is returned if @end_time has
 * passed.
 *
 * The following code shows how to correctly perform a timed wait on a
 * condition variable (extending the example presented in the
 * documentation for #SysCond):
 *
 * |[<!-- language="C" --> 
 * SysPointer
 * pop_data_timed (void)
 * {
 *   SysInt64 end_time;
 *   SysPointer data;
 *
 *   sys_mutex_lock (&data_mutex);
 *
 *   end_time = sys_get_monotonic_time () + 5 * SYS_TIME_SPAN_SECOND;
 *   while (!current_data)
 *     if (!sys_cond_wait_until (&data_cond, &data_mutex, end_time))
 *       {
 *         // timeout has passed.
 *         sys_mutex_unlock (&data_mutex);
 *         return NULL;
 *       }
 *
 *   // there is data for us
 *   data = current_data;
 *   current_data = NULL;
 *
 *   sys_mutex_unlock (&data_mutex);
 *
 *   return data;
 * }
 * ]|
 *
 * Notice that the end time is calculated once, before entering the
 * loop and reused.  This is the motivation behind the use of absolute
 * time on this API -- if a relative time of 5 seconds were passed
 * directly to the call and a spurious wakeup occurred, the program would
 * have to start over waiting again (which would lead to a total wait
 * time of more than 5 seconds).
 *
 * Returns: %true on a signal, %false on a timeout
 * Since: 2.32
 **/
SysBool
sys_cond_wait_until (SysCond  *cond,
                   SysMutex *mutex,
                   SysInt64  end_time)
{
  struct timespec ts;
  SysInt status;

  /* This is the exact check we used during init to set the clock to
   * monotonic, so if we're in this branch, timedwait() will already be
   * expecting a monotonic clock.
   */
  {
    ts.tv_sec = end_time / 1000000;
    ts.tv_nsec = (end_time % 1000000) * 1000;

    if ((status = pthread_cond_timedwait (sys_cond_get_impl (cond), sys_mutex_get_impl (mutex), &ts)) == 0)
      return true;
  }

  if SYS_UNLIKELY (status != ETIMEDOUT)
    sys_thread_abort (status, "pthread_cond_timedwait");

  return false;
}

#endif /* defined(USE_NATIVE_MUTEX) */

/* {{{1 SysPrivate */

/**
 * SysPrivate:
 *
 * The #SysPrivate struct is an opaque data structure to represent a
 * thread-local data key. It is approximately equivalent to the
 * pthread_setspecific()/pthread_getspecific() APIs on POSIX and to
 * TlsSetValue()/TlsGetValue() on Windows.
 *
 * If you don't already know why you might want this functionality,
 * then you probably don't need it.
 *
 * #SysPrivate is a very limited resource (as far as 128 per program,
 * shared between all libraries). It is also not possible to destroy a
 * #SysPrivate after it has been used. As such, it is only ever acceptable
 * to use #SysPrivate in static scope, and even then sparingly so.
 *
 * See SYS_PRIVATE_INIT() for a couple of examples.
 *
 * The #SysPrivate structure should be considered opaque.  It should only
 * be accessed via the sys_private_ functions.
 */

/**
 * SYS_PRIVATE_INIT:
 * @notify: a #SysDestroyFunc
 *
 * A macro to assist with the static initialisation of a #SysPrivate.
 *
 * This macro is useful for the case that a #SysDestroyFunc function
 * should be associated with the key.  This is needed when the key will be
 * used to point at memory that should be deallocated when the thread
 * exits.
 *
 * Additionally, the #SysDestroyFunc will also be called on the previous
 * value stored in the key when sys_private_replace() is used.
 *
 * If no #SysDestroyFunc is needed, then use of this macro is not
 * required -- if the #SysPrivate is declared in static scope then it will
 * be properly initialised by default (ie: to all zeros).  See the
 * examples below.
 *
 * |[<!-- language="C" --> 
 * static SysPrivate name_key = SYS_PRIVATE_INIT (sys_free);
 *
 * // return value should not be freed
 * const SysChar *
 * get_local_name (void)
 * {
 *   return sys_private_get (&name_key);
 * }
 *
 * void
 * set_local_name (const SysChar *name)
 * {
 *   sys_private_replace (&name_key, sys_strdup (name));
 * }
 *
 *
 * static SysPrivate count_key;   // no free function
 *
 * SysInt
 * get_local_count (void)
 * {
 *   return GPOINTER_TO_INT (sys_private_get (&count_key));
 * }
 *
 * void
 * set_local_count (SysInt count)
 * {
 *   sys_private_set (&count_key, GINT_TO_POINTER (count));
 * }
 * ]|
 *
 * Since: 2.32
 **/

static pthread_key_t *
sys_private_impl_new (SysDestroyFunc notify)
{
  pthread_key_t *key;
  SysInt status;

  key = malloc (sizeof (pthread_key_t));
  if SYS_UNLIKELY (key == NULL)
    sys_thread_abort (errno, "malloc");
  status = pthread_key_create (key, notify);
  if SYS_UNLIKELY (status != 0)
    sys_thread_abort (status, "pthread_key_create");

  return key;
}

static void
sys_private_impl_free (pthread_key_t *key)
{
  SysInt status;

  status = pthread_key_delete (*key);
  if SYS_UNLIKELY (status != 0)
    sys_thread_abort (status, "pthread_key_delete");
  free (key);
}

static inline pthread_key_t *
sys_private_get_impl (SysPrivate *key)
{
  pthread_key_t *impl = sys_atomic_pointer_get (&key->p);

  if SYS_UNLIKELY (impl == NULL)
    {
      impl = sys_private_impl_new (key->notify);
      if (!sys_atomic_pointer_cmpxchg (&key->p, NULL, impl))
        {
          sys_private_impl_free (impl);
          impl = key->p;
        }
    }

  return impl;
}

/**
 * sys_private_get:
 * @key: a #SysPrivate
 *
 * Returns the current value of the thread local variable @key.
 *
 * If the value has not yet been set in this thread, %NULL is returned.
 * Values are never copied between threads (when a new thread is
 * created, for example).
 *
 * Returns: the thread-local value
 */
SysPointer
sys_private_get (SysPrivate *key)
{
  /* quote POSIX: No errors are returned from pthread_getspecific(). */
  return pthread_getspecific (*sys_private_get_impl (key));
}

/**
 * sys_private_set:
 * @key: a #SysPrivate
 * @value: the new value
 *
 * Sets the thread local variable @key to have the value @value in the
 * current thread.
 *
 * This function differs from sys_private_replace() in the following way:
 * the #SysDestroyFunc for @key is not called on the old value.
 */
void
sys_private_set (SysPrivate *key,
               SysPointer  value)
{
  SysInt status;

  if SYS_UNLIKELY ((status = pthread_setspecific (*sys_private_get_impl (key), value)) != 0)
    sys_thread_abort (status, "pthread_setspecific");
}

/**
 * sys_private_replace:
 * @key: a #SysPrivate
 * @value: the new value
 *
 * Sets the thread local variable @key to have the value @value in the
 * current thread.
 *
 * This function differs from sys_private_set() in the following way: if
 * the previous value was non-%NULL then the #SysDestroyFunc handler for
 * @key is run on it.
 *
 * Since: 2.32
 **/
void
sys_private_replace (SysPrivate *key,
                   SysPointer  value)
{
  pthread_key_t *impl = sys_private_get_impl (key);
  SysPointer old;
  SysInt status;

  old = pthread_getspecific (*impl);

  if SYS_UNLIKELY ((status = pthread_setspecific (*impl, value)) != 0)
    sys_thread_abort (status, "pthread_setspecific");

  if (old && key->notify)
    key->notify (old);
}

/* {{{1 SysThread */

#define posix_check_err(err, name) SYS_STMT_START{			\
  SysInt error = (err); 							\
  if (error)	 		 		 			\
    sys_error_N ("file %s: line %d (%s): error '%s' during '%s'",		\
           sys_strerror (error), name);					\
  }SYS_STMT_END

#define posix_check_cmd(cmd) posix_check_err (cmd, #cmd)

typedef struct
{
  SysRealThread thread;

  pthread_t system_thread;
  SysBool  joined;
  SysMutex    lock;

  void *(*proxy) (void *);
} SysThreadPosix;

void
sys_system_thread_free (SysRealThread *thread)
{
  SysThreadPosix *pt = (SysThreadPosix *) thread;

  if (!pt->joined)
    pthread_detach (pt->system_thread);

  sys_mutex_clear (&pt->lock);

  sys_slice_free (SysThreadPosix, pt);
}

SysRealThread *
sys_system_thread_new (SysThreadFunc proxy,
                     SysULong stack_size,
                     const SysChar *name,
                     SysThreadFunc func,
                     SysPointer data,
                     SysError **error)
{
  SysThreadPosix *thread;
  SysRealThread *base_thread;
  pthread_attr_t attr;
  SysInt ret;

  thread = sys_slice_new0 (SysThreadPosix);
  base_thread = (SysRealThread*)thread;
  base_thread->ref_count = 2;
  base_thread->ours = true;
  base_thread->thread.joinable = true;
  base_thread->thread.func = func;
  base_thread->thread.data = data;
  base_thread->name = sys_strdup (name);
  thread->proxy = proxy;

  posix_check_cmd (pthread_attr_init (&attr));

#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
  if (stack_size)
    {
#ifdef _SC_THREAD_STACK_MIN
      long min_stack_size = sysconf (_SC_THREAD_STACK_MIN);
      if (min_stack_size >= 0)
        stack_size = MAX ((SysULong) min_stack_size, stack_size);
#endif /* _SC_THREAD_STACK_MIN */
      /* No error check here, because some systems can't do it and
       * we simply don't want threads to fail because of that. */
      pthread_attr_setstacksize (&attr, stack_size);
    }
#endif /* HAVE_PTHREAD_ATTR_SETSTACKSIZE */

#ifdef HAVE_PTHREAD_ATTR_SETINHERITSCHED
    {
      /* While this is the default, better be explicit about it */
      pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED);
    }
#endif /* HAVE_PTHREAD_ATTR_SETINHERITSCHED */

  ret = pthread_create (&thread->system_thread, &attr, (void* (*)(void*))proxy, thread);

  posix_check_cmd (pthread_attr_destroy (&attr));

  if (ret == EAGAIN)
    {
      sys_error_set_N (error, "Error creating thread: %s", sys_strerror (ret));
      sys_free (thread->thread.name);
      sys_slice_free (SysThreadPosix, thread);
      return NULL;
    }

  posix_check_err (ret, "pthread_create");

  sys_mutex_init (&thread->lock);

  return (SysRealThread *) thread;
}

/**
 * sys_thread_yield:
 *
 * Causes the calling thread to voluntarily relinquish the CPU, so
 * that other threads can run.
 *
 * This function is often used as a method to make busy wait less evil.
 */
void
sys_thread_yield (void)
{
  sched_yield ();
}

void
sys_system_thread_wait (SysRealThread *thread)
{
  SysThreadPosix *pt = (SysThreadPosix *) thread;

  sys_mutex_lock (&pt->lock);

  if (!pt->joined)
    {
      posix_check_cmd (pthread_join (pt->system_thread, NULL));
      pt->joined = true;
    }

  sys_mutex_unlock (&pt->lock);
}

void
sys_system_thread_exit (void)
{
  pthread_exit (NULL);
}

void
sys_system_thread_set_name (const SysChar *name)
{
#if defined(HAVE_PTHREAD_SETNAME_NP_WITHOUT_TID)
  pthread_setname_np (name); /* on OS X and iOS */
#elif defined(HAVE_PTHREAD_SETNAME_NP_WITH_TID)
  pthread_setname_np (pthread_self (), name); /* on Linux and Solaris */
#elif defined(HAVE_PTHREAD_SETNAME_NP_WITH_TID_AND_ARG)
  pthread_setname_np (pthread_self (), "%s", (SysChar *) name); /* on NetBSD */
#elif defined(HAVE_PTHREAD_SET_NAME_NP)
  pthread_set_name_np (pthread_self (), name); /* on FreeBSD, DragonFlyBSD, OpenBSD */
#endif
}

/* {{{1 SysMutex and SysCond futex implementation */

#if defined(USE_NATIVE_MUTEX)
/* We should expand the set of operations available in gatomic once we
 * have better C11 support in GCC in common distributions (ie: 4.9).
 *
 * Before then, let's define a couple of useful things for our own
 * purposes...
 */

#ifdef HAVE_STDATOMIC_H

#include <stdatomic.h>

#define exchange_acquire(ptr, new) \
  atomic_exchange_explicit((atomic_uint *) (ptr), (new), __ATOMIC_ACQUIRE)
#define compare_exchange_acquire(ptr, old, new) \
  atomic_compare_exchange_stronsys_explicit((atomic_uint *) (ptr), (old), (new), \
                                          __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)

#define exchange_release(ptr, new) \
  atomic_exchange_explicit((atomic_uint *) (ptr), (new), __ATOMIC_RELEASE)
#define store_release(ptr, new) \
  atomic_store_explicit((atomic_uint *) (ptr), (new), __ATOMIC_RELEASE)

#else

#define exchange_acquire(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_ACQUIRE)
#define compare_exchange_acquire(ptr, old, new) \
  __atomic_compare_exchange_4((ptr), (old), (new), 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)

#define exchange_release(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_RELEASE)
#define store_release(ptr, new) \
  __atomic_store_4((ptr), (new), __ATOMIC_RELEASE)

#endif

/* Our strategy for the mutex is pretty simple:
 *
 *  0: not in use
 *
 *  1: acquired by one thread only, no contention
 *
 *  2: contended
 */

typedef enum {
  SYS_MUTEX_STATE_EMPTY = 0,
  SYS_MUTEX_STATE_OWNED,
  SYS_MUTEX_STATE_CONTENDED,
} SysMutexState;

 /*
 * As such, attempting to acquire the lock should involve an increment.
 * If we find that the previous value was 0 then we can return
 * immediately.
 *
 * On unlock, we always store 0 to indicate that the lock is available.
 * If the value there was 1 before then we didn't have contention and
 * can return immediately.  If the value was something other than 1 then
 * we have the contended case and need to wake a waiter.
 *
 * If it was not 0 then there is another thread holding it and we must
 * wait.  We must always ensure that we mark a value >1 while we are
 * waiting in order to instruct the holder to do a wake operation on
 * unlock.
 */

void
sys_mutex_init (SysMutex *mutex)
{
  mutex->i[0] = SYS_MUTEX_STATE_EMPTY;
}

void
sys_mutex_clear (SysMutex *mutex)
{
  if SYS_UNLIKELY (mutex->i[0] != SYS_MUTEX_STATE_EMPTY)
    {
      fprintf (stderr, "sys_mutex_clear() called on uninitialised or locked mutex\n");
      sys_abort_N_N_N_N ();
    }
}

SYS_GNUC_NO_INLINE
static void
sys_mutex_lock_slowpath (SysMutex *mutex)
{
  /* Set to contended.  If it was empty before then we
   * just acquired the lock.
   *
   * Otherwise, sleep for as long as the contended state remains...
   */
  while (exchange_acquire (&mutex->i[0], SYS_MUTEX_STATE_CONTENDED) != SYS_MUTEX_STATE_EMPTY)
    {
      sys_futex_simple (&mutex->i[0], (SysSize) FUTEX_WAIT_PRIVATE,
                      SYS_MUTEX_STATE_CONTENDED, NULL);
    }
}

SYS_GNUC_NO_INLINE
static void
sys_mutex_unlock_slowpath (SysMutex *mutex,
                         SysUInt   prev)
{
  /* We seem to get better code for the uncontended case by splitting
   * this out...
   */
  if SYS_UNLIKELY (prev == SYS_MUTEX_STATE_EMPTY)
    {
      fprintf (stderr, "Attempt to unlock mutex that was not locked\n");
      sys_abort_N_N_N_N ();
    }

  sys_futex_simple (&mutex->i[0], (SysSize) FUTEX_WAKE_PRIVATE, (SysSize) 1, NULL);
}

void
sys_mutex_lock (SysMutex *mutex)
{
  /* empty -> owned and we're done.  Anything else, and we need to wait... */
  if SYS_UNLIKELY (!sys_atomic_int_cmpxchg (&mutex->i[0],
                                                     SYS_MUTEX_STATE_EMPTY,
                                                     SYS_MUTEX_STATE_OWNED))
    sys_mutex_lock_slowpath (mutex);
}

void
sys_mutex_unlock (SysMutex *mutex)
{
  SysUInt prev;

  prev = exchange_release (&mutex->i[0], SYS_MUTEX_STATE_EMPTY);

  /* 1-> 0 and we're done.  Anything else and we need to signal... */
  if SYS_UNLIKELY (prev != SYS_MUTEX_STATE_OWNED)
    sys_mutex_unlock_slowpath (mutex, prev);
}

SysBool
sys_mutex_trylock (SysMutex *mutex)
{
  SysMutexState empty = SYS_MUTEX_STATE_EMPTY;

  /* We don't want to touch the value at all unless we can move it from
   * exactly empty to owned.
   */
  return compare_exchange_acquire (&mutex->i[0], &empty, SYS_MUTEX_STATE_OWNED);
}

/* Condition variables are implemented in a rather simple way as well.
 * In many ways, futex() as an abstraction is even more ideally suited
 * to condition variables than it is to mutexes.
 *
 * We store a generation counter.  We sample it with the lock held and
 * unlock before sleeping on the futex.
 *
 * Signalling simply involves increasing the counter and making the
 * appropriate futex call.
 *
 * The only thing that is the slightest bit complicated is timed waits
 * because we must convert our absolute time to relative.
 */

void
sys_cond_init (SysCond *cond)
{
  cond->i[0] = 0;
}

void
sys_cond_clear (SysCond *cond)
{
}

void
sys_cond_wait (SysCond  *cond,
             SysMutex *mutex)
{
  SysUInt sampled = (SysUInt) sys_atomic_int_get (&cond->i[0]);

  sys_mutex_unlock (mutex);
  sys_futex_simple (&cond->i[0], (SysSize) FUTEX_WAIT_PRIVATE, (SysSize) sampled, NULL);
  sys_mutex_lock (mutex);
}

void
sys_cond_signal (SysCond *cond)
{
  sys_atomic_int_inc (&cond->i[0]);

  sys_futex_simple (&cond->i[0], (SysSize) FUTEX_WAKE_PRIVATE, (SysSize) 1, NULL);
}

void
sys_cond_broadcast (SysCond *cond)
{
  sys_atomic_int_inc (&cond->i[0]);

  sys_futex_simple (&cond->i[0], (SysSize) FUTEX_WAKE_PRIVATE, (SysSize) INT_MAX, NULL);
}

SysBool
sys_cond_wait_until (SysCond  *cond,
                   SysMutex *mutex,
                   SysInt64  end_time)
{
  struct timespec now;
  struct timespec span;

  SysUInt sampled;
  SysInt res;
  SysBool success;

  if (end_time < 0)
    return false;

  clock_gettime (CLOCK_MONOTONIC, &now);
  span.tv_sec = (end_time / 1000000) - now.tv_sec;
  span.tv_nsec = ((end_time % 1000000) * 1000) - now.tv_nsec;
  if (span.tv_nsec < 0)
    {
      span.tv_nsec += 1000000000;
      span.tv_sec--;
    }

  if (span.tv_sec < 0)
    return false;

  /* `struct timespec` as defined by the libc headers does not necessarily
   * have any relation to the one used by the kernel for the `futex` syscall.
   *
   * Specifically, the libc headers might use 64-bit `time_t` while the kernel
   * headers use 32-bit `__kernel_old_time_t` on certain systems.
   *
   * To get around this problem we
   *   a) check if `futex_time64` is available, which only exists on 32-bit
   *      platforms and always uses 64-bit `time_t`.
   *   b) otherwise (or if that returns `ENOSYS`), we call the normal `futex`
   *      syscall with the `struct timespec` used by the kernel, which uses
   *      `__kernel_lonsys_t` for both its fields. We use that instead of
   *      `__kernel_old_time_t` because it is equivalent and available in the
   *      kernel headers for a longer time.
   *
   * Also some 32-bit systems do not define `__NR_futex` at all and only
   * define `__NR_futex_time64`.
   */

  sampled = cond->i[0];
  sys_mutex_unlock (mutex);

#ifdef __NR_futex_time64
  {
    struct
    {
      SysInt64 tv_sec;
      SysInt64 tv_nsec;
    } span_arg;

    span_arg.tv_sec = span.tv_sec;
    span_arg.tv_nsec = span.tv_nsec;

    res = syscall (__NR_futex_time64, &cond->i[0], (SysSize) FUTEX_WAIT_PRIVATE, (SysSize) sampled, &span_arg);

    /* If the syscall does not exist (`ENOSYS`), we retry again below with the
     * normal `futex` syscall. This can happen if newer kernel headers are
     * used than the kernel that is actually running.
     */
#  ifdef __NR_futex
    if (res >= 0 || errno != ENOSYS)
#  endif /* defined(__NR_futex) */
      {
        success = (res < 0 && errno == ETIMEDOUT) ? false : true;
        sys_mutex_lock (mutex);

        return success;
      }
  }
#endif

#ifdef __NR_futex
  {
    struct
    {
      __kernel_lonsys_t tv_sec;
      __kernel_lonsys_t tv_nsec;
    } span_arg;

    /* Make sure to only ever call this if the end time actually fits into the target type */
    if (SYS_UNLIKELY (sizeof (__kernel_lonsys_t) < 8 && span.tv_sec > SYS_MAXINT32))
      sys_error ("%s: Can’t wait for more than %us", SYS_STRFUNC, SYS_MAXINT32);

    span_arg.tv_sec = span.tv_sec;
    span_arg.tv_nsec = span.tv_nsec;

    res = syscall (__NR_futex, &cond->i[0], (SysSize) FUTEX_WAIT_PRIVATE, (SysSize) sampled, &span_arg);
    success = (res < 0 && errno == ETIMEDOUT) ? false : true;
    sys_mutex_lock (mutex);

    return success;
  }
#endif /* defined(__NR_futex) */

  /* We can't end up here because of the checks above */
  sys_assert_not_reached ();
}

#endif

  /* {{{1 Epilogue */
/* vim:set foldmethod=marker: */
