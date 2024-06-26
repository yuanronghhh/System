#include <System/Platform/Common/SysThreadPrivate.h>
#include <System/DataTypes/SysSList.h>
#include <System/Utils/SysString.h>

/**
 * this code from glib Thread
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

static SysMutex    sys_once_mutex;
static SysCond     sys_once_cond;
static SysSList   *sys_once_init_list = NULL;

static SysInt sys_thread_n_created_counter = 0;  /* (atomic) */

static void sys_thread_cleanup (SysPointer data);
static SysPrivate     sys_thread_specific_private = SYS_PRIVATE_INIT (sys_thread_cleanup);

/* SysOnce {{{1 ------------------------------------------------------------- */
SysPointer sys_once_impl (SysOnce *once, SysThreadFunc func, SysPointer arg) {
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

SysBool _sys_once_init_enter (volatile SysPointer location) {
  volatile SysPointer value_location = location;
  SysBool need_init = false;
  sys_mutex_lock (&sys_once_mutex);

  if (sys_atomic_pointer_get (value_location) == NULL) {

    if (!sys_slist_find (sys_once_init_list, (void*) value_location)) {
      need_init = true;
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

void _sys_once_init_leave (volatile SysPointer location, SysSize          result) {
  volatile SysPointer value_location = location;

  sys_return_if_fail (sys_atomic_pointer_get (value_location) == NULL);
  sys_return_if_fail (result != 0);
  sys_return_if_fail (sys_once_init_list != NULL);

  sys_atomic_pointer_set (value_location, (SysPointer)result);
  sys_mutex_lock (&sys_once_mutex);
  sys_once_init_list = sys_slist_remove (sys_once_init_list, (void*) value_location);
  sys_cond_broadcast (&sys_once_cond);
  sys_mutex_unlock (&sys_once_mutex);
}

/* SysThread  */
void sys_thread_init(void) {
  sys_system_thread_init();
}

void sys_thread_detach(void) {
  sys_system_thread_detach();
}

SysThread * sys_thread_ref (SysThread *thread) {
  SysRealThread *real = (SysRealThread *) thread;

  sys_atomic_int_inc (&real->ref_count);

  return thread;
}

void sys_thread_unref (SysThread *thread) {
  SysRealThread *real = (SysRealThread *) thread;

  if (sys_atomic_int_dec_and_test (&real->ref_count))
    {
      if (real->ours)
        sys_system_thread_free (real);
      else
        sys_slice_free (SysRealThread, real);
    }
}

static void sys_thread_cleanup (SysPointer data) {
  sys_thread_unref (data);
}

SysPointer sys_thread_proxy (SysPointer data) {
  SysRealThread* thread = data;

  sys_assert (data);
  sys_private_set (&sys_thread_specific_private, data);

  if (thread->name)
    {
      sys_system_thread_set_name (thread->name);
      sys_free (thread->name);
      thread->name = NULL;
    }

  thread->retval = thread->thread.func (thread->thread.data);

  return NULL;
}

SysUInt sys_thread_n_created (void) {
  return sys_atomic_int_get (&sys_thread_n_created_counter);
}

SysThread * sys_thread_new (const SysChar *name, SysThreadFunc  func, SysPointer     data) {
  SysError *error = NULL;
  SysThread *thread;

  thread = sys_thread_new_internal (name, sys_thread_proxy, func, data, 0, &error);

  if SYS_UNLIKELY (thread == NULL)
    sys_error_N ("creating thread '%s': %s", name ? name : "", error->message);

  return thread;
}

SysThread * sys_thread_try_new (const SysChar  *name,
                  SysThreadFunc   func,
                  SysPointer      data,
                  SysError      **error) {
  return sys_thread_new_internal (name, sys_thread_proxy, func, data, 0, error);
}

SysThread * sys_thread_new_internal (const SysChar *name,
                       SysThreadFunc proxy,
                       SysThreadFunc func,
                       SysPointer data,
                       SysSize stack_size,
                       SysError **error) {
  sys_return_val_if_fail (func != NULL, NULL);

  sys_atomic_int_inc (&sys_thread_n_created_counter);

  return (SysThread *) sys_system_thread_new (proxy, stack_size, name, func, data, error);
}

void sys_thread_exit (SysPointer retval) {
  SysRealThread* real = (SysRealThread*) sys_thread_self ();

  if SYS_UNLIKELY (!real->ours)
    sys_error_N ("%s", "attempt to sys_thread_exit() a thread not created by GLib");

  real->retval = retval;

  sys_system_thread_exit ();
}

SysPointer sys_thread_join (SysThread *thread) {
  SysRealThread *real = (SysRealThread*) thread;
  SysPointer retval;

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

  if (!thread)
    {
      /* If no thread data is available, provide and set one.
       * This can happen for the main thread and for threads
       * that are not created by GLib.
       */
      thread = sys_slice_new0 (SysRealThread);
      thread->ref_count = 1;

      sys_private_set (&sys_thread_specific_private, thread);
    }

  return (SysThread*) thread;
}

SysUInt sys_get_num_processors (void) {
#ifdef SYS_OS_WIN32
  SysUInt count;
  SYSTEM_INFO sysinfo;
  DWORD_PTR process_cpus;
  DWORD_PTR system_cpus;

  /* This *never* fails, use it as fallback */
  GetNativeSystemInfo (&sysinfo);
  count = (SysInt) sysinfo.dwNumberOfProcessors;

  if (GetProcessAffinityMask (GetCurrentProcess (),
                              &process_cpus, &system_cpus))
    {
      SysUInt af_count;

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
    SysInt count;

    count = sysconf (_SC_NPROCESSORS_ONLN);
    if (count > 0)
      return count;
  }
#elif defined HW_NCPU
  {
    SysInt mib[2], count = 0;
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

