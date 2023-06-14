#include <System/Platform/Common/SysThreadPrivate.h>


static SysMutex    sys_once_mutex;
static SysCond     sys_once_cond;
static SysSList   *sys_once_init_list = NULL;

static void sys_thread_cleanup (gpointer data);
static GPrivate     sys_thread_specific_private = SYS_PRIVATE_INIT (sys_thread_cleanup);

SYS_LOCK_DEFINE_STATIC (sys_thread_new);

/* SysOnce {{{1 ------------------------------------------------------------- */
gpointer sys_once_impl (SysOnce *once, SysThreadFunc func, gpointer arg) {
  sys_mutex_lock (&sys_once_mutex);

  while (once->status == SYS_ONCE_STATUS_PROGRESS)
    sys_cond_wait (&sys_once_cond, &sys_once_mutex);

  if (once->status != SYS_ONCE_STATUS_READY) {
    once->status = SYS_ONCE_STATUS_PROGRESS;
    sys_mutex_unlock (&sys_once_mutex);

    once->retval = func (arg);

    sys_mutex_lock (&sys_once_mutex);
    once->status = SYS_ONCE_STATUS_READY;
    sys_cond_broadcast (&sys_once_cond);
  }

  sys_mutex_unlock (&sys_once_mutex);

  return once->retval;
}

SysBool (sys_once_init_enter) (volatile void *location) {
  volatile gsize *value_location = location;
  SysBool need_init = false;
  sys_mutex_lock (&sys_once_mutex);

  if (sys_atomic_pointer_get (value_location) == NULL) {

    if (!sys_slist_find (sys_once_init_list, (void*) value_location)) {
      need_init = TRUE;
      sys_once_init_list = sys_slist_prepend (sys_once_init_list, (void*) value_location);

    } else {

      do {
        sys_cond_wait (&sys_once_cond, &sys_once_mutex);
      } while (sys_slist_find (sys_once_init_list, (void*) value_location));
    }
  }

  sys_mutex_unlock (&sys_once_mutex);

  return need_init;
}

void (sys_once_init_leave) (volatile void *location, gsize          result) {
  volatile gsize *value_location = location;

  sys_return_if_fail (sys_atomic_pointer_get (value_location) == NULL);
  sys_return_if_fail (result != 0);
  sys_return_if_fail (sys_once_init_list != NULL);

  sys_atomic_pointer_set (value_location, result);
  sys_mutex_lock (&sys_once_mutex);
  sys_once_init_list = sys_slist_remove (sys_once_init_list, (void*) value_location);
  sys_cond_broadcast (&sys_once_cond);
  sys_mutex_unlock (&sys_once_mutex);
}

/* SysThread  */

SysThread * sys_thread_ref (SysThread *thread) {
  SysRealThread *real = (SysRealThread *) thread;

  sys_atomic_int_inc (&real->ref_count);

  return thread;
}

void sys_thread_unref (SysThread *thread) {
  SysRealThread *real = (SysRealThread *) thread;

  if (sys_atomic_int_dec_and_test (&real->ref_count)) {
    if (real->ours) {
      sys_system_thread_free (real);

    } else {
      sys_slice_free (SysRealThread, real);
    }
  }
}

static void sys_thread_cleanup (gpointer data) {
  sys_thread_unref (data);
}

gpointer sys_thread_proxy (gpointer data) {
  SysRealThread* thread = data;

  sys_assert (data);

  /* This has to happen before SYS_LOCK, as that might call sys_thread_self */
  sys_private_set (&sys_thread_specific_private, data);

  /* The lock makes sure that sys_thread_new_internal() has a chance to
   * setup 'func' and 'data' before we make the call.
   */
  SYS_LOCK (sys_thread_new);
  SYS_UNLOCK (sys_thread_new);

  TRACE (GLIB_THREAD_SPAWNED (thread->thread.func, thread->thread.data,
        thread->name));

  if (thread->name)
  {
    sys_system_thread_set_name (thread->name);
    sys_free (thread->name);
    thread->name = NULL;
  }

  thread->retval = thread->thread.func (thread->thread.data);

  return NULL;
}

SysThread * sys_thread_new (const gchar *name, SysThreadFunc  func, gpointer     data) {
  GError *error = NULL;
  SysThread *thread;

  thread = sys_thread_new_internal (name, sys_thread_proxy, func, data, 0, &error);

  if SYS_UNLIKELY (thread == NULL)
    sys_error ("creating thread '%s': %s", name ? name : "", error->message);

  return thread;
}

SysThread * sys_thread_try_new (const gchar  *name, SysThreadFunc   func, gpointer      data, GError      **error) {
  return sys_thread_new_internal (name, sys_thread_proxy, func, data, 0, error);
}

SysThread * sys_thread_new_internal (const gchar   *name, SysThreadFunc    proxy, SysThreadFunc    func, gpointer       data, gsize          stack_size, GError       **error) {
  SysRealThread *thread;

  sys_return_val_if_fail (func != NULL, NULL);

  SYS_LOCK (sys_thread_new);
  thread = sys_system_thread_new (proxy, stack_size, error);
  if (thread) {
    thread->ref_count = 2;
    thread->ours = TRUE;
    thread->thread.joinable = TRUE;
    thread->thread.func = func;
    thread->thread.data = data;
    thread->name = sys_strdup (name);
  }
  SYS_UNLOCK (sys_thread_new);

  return (SysThread*) thread;
}

void sys_thread_exit (gpointer retval) {
  SysRealThread* real = (SysRealThread*) sys_thread_self ();

  if SYS_UNLIKELY (!real->ours)
    sys_error ("attempt to sys_thread_exit() a thread not created by GLib");

  real->retval = retval;

  sys_system_thread_exit ();
}

gpointer sys_thread_join (SysThread *thread) {
  SysRealThread *real = (SysRealThread*) thread;
  gpointer retval;

  sys_return_val_if_fail (thread, NULL);
  sys_return_val_if_fail (real->ours, NULL);

  sys_system_thread_wait (real);

  retval = real->retval;

  /* Just to make sure, this isn't used any more */
  thread->joinable = 0;

  sys_thread_unref (thread);

  return retval;
}

SysThread* sys_thread_self (void) {
  SysRealThread* thread = sys_private_get (&sys_thread_specific_private);

  if (!thread) {
    thread = sys_slice_new0 (SysRealThread);
    thread->ref_count = 1;

    sys_private_set (&sys_thread_specific_private, thread);
  }

  return (SysThread*) thread;
}

guint sys_get_num_processors (void) {
#ifdef SYS_OS_WIN32
  unsigned int count;
  SYSTEM_INFO sysinfo;
  DWORD_PTR process_cpus;
  DWORD_PTR system_cpus;

  /* This *never* fails, use it as fallback */
  GetNativeSystemInfo (&sysinfo);
  count = (int) sysinfo.dwNumberOfProcessors;

  if (GetProcessAffinityMask (GetCurrentProcess (),
        &process_cpus, &system_cpus))
  {
    unsigned int af_count;

    for (af_count = 0; process_cpus != 0; process_cpus >>= 1)
      if (process_cpus & 1)
        af_count++;

    /* Prefer affinity-based result, if available */
    if (af_count > 0)
      count = af_count;
  }

  if (count > 0)
    return count;
#elif defined(_SC_NPROCESSORS_ONLN)
  {
    int count;

    count = sysconf (_SC_NPROCESSORS_ONLN);
    if (count > 0)
      return count;
  }
#elif defined HW_NCPU
  {
    int mib[2], count = 0;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(count);

    if (sysctl (mib, 2, &count, &len, NULL, 0) == 0 && count > 0)
      return count;
  }
#endif

  return 1; /* Fallback */
}
