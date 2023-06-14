#include <System/Platform/Common/SysThreadPrivate.h>


static void sys_thread_abort (SysInt status, const SysChar *function) {
  sys_abort_N ("GLib (gthread-win32.c): Unexpected error from C library during '%s': %s.  Aborting.\n",
      sys_strerr (status), function);
}

/* {{{1 SysMutex */
void sys_mutex_init (SysMutex *mutex)
{
  InitializeSRWLock ((SysPointer) mutex);
}

void sys_mutex_clear (SysMutex *mutex)
{
}

void sys_mutex_lock (SysMutex *mutex)
{
  AcquireSRWLockExclusive ((SysPointer) mutex);
}

SysBool sys_mutex_trylock (SysMutex *mutex)
{
  return TryAcquireSRWLockExclusive ((SysPointer) mutex);
}

void sys_mutex_unlock (SysMutex *mutex)
{
  ReleaseSRWLockExclusive ((SysPointer) mutex);
}

/* {{{1 GRecMutex */

static CRITICAL_SECTION * sys_rec_mutex_impl_new (void)
{
  CRITICAL_SECTION *cs;

  cs = sys_slice_new (CRITICAL_SECTION);
  InitializeCriticalSection (cs);

  return cs;
}

static void sys_rec_mutex_impl_free (CRITICAL_SECTION *cs)
{
  DeleteCriticalSection (cs);
  sys_slice_free (CRITICAL_SECTION, cs);
}

static CRITICAL_SECTION * sys_rec_mutex_get_impl (GRecMutex *mutex)
{
  CRITICAL_SECTION *impl = mutex->p;

  if SYS_UNLIKELY (mutex->p == NULL)
  {
    impl = sys_rec_mutex_impl_new ();
    if (InterlockedCompareExchangePointer (&mutex->p, impl, NULL) != NULL)
      sys_rec_mutex_impl_free (impl);
    impl = mutex->p;
  }

  return impl;
}

void sys_rec_mutex_init (GRecMutex *mutex)
{
  mutex->p = sys_rec_mutex_impl_new ();
}

void sys_rec_mutex_clear (GRecMutex *mutex)
{
  sys_rec_mutex_impl_free (mutex->p);
}

void sys_rec_mutex_lock (GRecMutex *mutex)
{
  EnterCriticalSection (sys_rec_mutex_get_impl (mutex));
}

void sys_rec_mutex_unlock (GRecMutex *mutex)
{
  LeaveCriticalSection (mutex->p);
}

SysBool sys_rec_mutex_trylock (GRecMutex *mutex)
{
  return TryEnterCriticalSection (sys_rec_mutex_get_impl (mutex));
}

/* {{{1 GRWLock */

void sys_rw_lock_init (GRWLock *lock)
{
  InitializeSRWLock ((SysPointer) lock);
}

void sys_rw_lock_clear (GRWLock *lock)
{
}

void sys_rw_lock_writer_lock (GRWLock *lock)
{
  AcquireSRWLockExclusive ((SysPointer) lock);
}

SysBool sys_rw_lock_writer_trylock (GRWLock *lock)
{
  return TryAcquireSRWLockExclusive ((SysPointer) lock);
}

void sys_rw_lock_writer_unlock (GRWLock *lock)
{
  ReleaseSRWLockExclusive ((SysPointer) lock);
}

void sys_rw_lock_reader_lock (GRWLock *lock)
{
  AcquireSRWLockShared ((SysPointer) lock);
}

SysBool sys_rw_lock_reader_trylock (GRWLock *lock)
{
  return TryAcquireSRWLockShared ((SysPointer) lock);
}

void sys_rw_lock_reader_unlock (GRWLock *lock)
{
  ReleaseSRWLockShared ((SysPointer) lock);
}

/* {{{1 GCond */
void sys_cond_init (GCond *cond)
{
  InitializeConditionVariable ((SysPointer) cond);
}

void sys_cond_clear (GCond *cond)
{
}

void sys_cond_signal (GCond *cond)
{
  WakeConditionVariable ((SysPointer) cond);
}

void sys_cond_broadcast (GCond *cond)
{
  WakeAllConditionVariable ((SysPointer) cond);
}

void sys_cond_wait (GCond  *cond, SysMutex *entered_mutex)
{
  SleepConditionVariableSRW ((SysPointer) cond, (SysPointer) entered_mutex, INFINITE, 0);
}

SysBool sys_cond_wait_until (GCond  *cond, SysMutex *entered_mutex, SysInt64  end_time)
{
  SysInt64 span, start_time;
  DWORD span_millis;
  SysBool signalled;

  start_time = sys_get_monotonic_time ();
  do
  {
    span = end_time - start_time;

    if SYS_UNLIKELY (span < 0)
      span_millis = 0;
    else if SYS_UNLIKELY (span > SYS_GINT64_CONSTANT (1000) * (DWORD) INFINITE)
      span_millis = INFINITE;
    else
      /* Round up so we don't time out too early */
      span_millis = (span + 1000 - 1) / 1000;

    /* We never want to wait infinitely */
    if (span_millis >= INFINITE)
      span_millis = INFINITE - 1;

    signalled = SleepConditionVariableSRW ((SysPointer) cond, (SysPointer) entered_mutex, span_millis, 0);
    if (signalled)
      break;

    /* In case we didn't wait long enough after a timeout, wait again for the
     * remaining time */
    start_time = sys_get_monotonic_time ();
  }
  while (start_time < end_time);

  return signalled;
}

/* {{{1 GPrivate */

typedef struct _GPrivateDestructor GPrivateDestructor;

struct _GPrivateDestructor
{
  DWORD               index;
  GDestroyNotify      notify;
  GPrivateDestructor *next;
};

static GPrivateDestructor *sys_private_destructors;  /* (atomic) prepend-only */
static CRITICAL_SECTION sys_private_lock;

static DWORD sys_private_get_impl (GPrivate *key)
{
  DWORD impl = (DWORD) GPOINTER_TO_UINT(key->p);

  if SYS_UNLIKELY (impl == 0)
  {
    EnterCriticalSection (&sys_private_lock);
    impl = (UINT_PTR) key->p;
    if (impl == 0)
    {
      GPrivateDestructor *destructor;

      impl = TlsAlloc ();

      if SYS_UNLIKELY (impl == 0)
      {
        /* Ignore TLS index 0 temporarily (as 0 is the indicator that we
         * haven't allocated TLS yet) and alloc again;
         * See https://gitlab.gnome.org/GNOME/glib/-/issues/2058 */
        DWORD impl2 = TlsAlloc ();
        TlsFree (impl);
        impl = impl2;
      }

      if (impl == TLS_OUT_OF_INDEXES || impl == 0)
        sys_thread_abort (0, "TlsAlloc");

      if (key->notify != NULL)
      {
        destructor = malloc (sizeof (GPrivateDestructor));
        if SYS_UNLIKELY (destructor == NULL)
          sys_thread_abort (errno, "malloc");
        destructor->index = impl;
        destructor->notify = key->notify;
        destructor->next = sys_atomic_pointer_get (&sys_private_destructors);

        /* We need to do an atomic store due to the unlocked
         * access to the destructor list from the thread exit
         * function.
         *                * It can double as a sanity check...
         */
        if (!sys_atomic_pointer_compare_and_exchange (&sys_private_destructors,
              destructor->next,
              destructor))
          sys_thread_abort (0, "sys_private_get_impl(1)");
      }

      /* Ditto, due to the unlocked access on the fast path */
      if (!sys_atomic_pointer_compare_and_exchange (&key->p, NULL, impl))
        sys_thread_abort (0, "sys_private_get_impl(2)");
    }
    LeaveCriticalSection (&sys_private_lock);
  }

  return impl;
}

SysPointer sys_private_get (GPrivate *key)
{
  return TlsGetValue (sys_private_get_impl (key));
}

void sys_private_set (GPrivate *key, SysPointer  value)
{
  TlsSetValue (sys_private_get_impl (key), value);
}

void sys_private_replace (GPrivate *key, SysPointer  value)
{
  DWORD impl = sys_private_get_impl (key);
  SysPointer old;

  old = TlsGetValue (impl);
  TlsSetValue (impl, value);
  if (old && key->notify)
    key->notify (old);
}

/* {{{1 GThread */

#define win32_check_for_error(what) SYS_STMT_START{			\
  if (!(what))								\
  sys_error ("file %s: line %d (%s): error %s during %s",		\
      __FILE__, __LINE__, SYS_STRFUNC,				\
      sys_win32_error_message (GetLastError ()), #what);		\
}SYS_STMT_END

#define SYS_MUTEX_SIZE (sizeof (SysPointer))

typedef BOOL (__stdcall *GTryEnterCriticalSectionFunc) (CRITICAL_SECTION *);

typedef struct
{
  GRealThread thread;

  GThreadFunc proxy;
  HANDLE      handle;
} GThreadWin32;

void sys_system_thread_free (GRealThread *thread)
{
  GThreadWin32 *wt = (GThreadWin32 *) thread;

  win32_check_for_error (CloseHandle (wt->handle));
  sys_slice_free (GThreadWin32, wt);
}

void sys_system_thread_exit (void)
{
  _endthreadex (0);
}

static guint __stdcall sys_thread_win32_proxy (SysPointer data)
{
  GThreadWin32 *self = data;

  self->proxy (self);

  sys_system_thread_exit ();

  sys_assert_not_reached ();

  return 0;
}

SysBool sys_system_thread_get_scheduler_settings (GThreadSchedulerSettings *scheduler_settings)
{
  HANDLE current_thread = GetCurrentThread ();
  scheduler_settings->thread_prio = GetThreadPriority (current_thread);

  return TRUE;
}

GRealThread * sys_system_thread_new (GThreadFunc proxy, gulong stack_size, const GThreadSchedulerSettings *scheduler_settings, const char *name, GThreadFunc func, SysPointer data, GError **error)
{
  GThreadWin32 *thread;
  GRealThread *base_thread;
  guint ignore;
  const SysChar *message = NULL;
  int thread_prio;

  thread = sys_slice_new0 (GThreadWin32);
  thread->proxy = proxy;
  thread->handle = (HANDLE) NULL;
  base_thread = (GRealThread*)thread;
  base_thread->ref_count = 2;
  base_thread->ours = TRUE;
  base_thread->thread.joinable = TRUE;
  base_thread->thread.func = func;
  base_thread->thread.data = data;
  base_thread->name = sys_strdup (name);

  thread->handle = (HANDLE) _beSysInthreadex (NULL, stack_size, sys_thread_win32_proxy, thread,
      CREATE_SUSPENDED, &ignore);

  if (thread->handle == NULL)
  {
    message = "Error creating thread";
    goto error;
  }

  /* For thread priority inheritance we need to manually set the thread
   * priority of the new thread to the priority of the current thread. We
   * also have to start the thread suspended and resume it after actually
   * setting the priority here.
   *    * On Windows, by default all new threads are created with NORMAL thread
   * priority.
   */

  if (scheduler_settings)
  {
    thread_prio = scheduler_settings->thread_prio;
  }
  else
  {
    HANDLE current_thread = GetCurrentThread ();
    thread_prio = GetThreadPriority (current_thread);
  }

  if (thread_prio == THREAD_PRIORITY_ERROR_RETURN)
  {
    message = "Error getting current thread priority";
    goto error;
  }

  if (SetThreadPriority (thread->handle, thread_prio) == 0)
  {
    message = "Error setting new thread priority";
    goto error;
  }

  if (ResumeThread (thread->handle) == -1)
  {
    message = "Error resuming new thread";
    goto error;
  }

  return (GRealThread *) thread;

error:
  {
    SysChar *win_error = sys_win32_error_message (GetLastError ());
    sys_set_error (error, SYS_THREAD_ERROR, SYS_THREAD_ERROR_AGAIN,
        "%s: %s", message, win_error);
    sys_free (win_error);
    if (thread->handle)
      CloseHandle (thread->handle);
    sys_slice_free (GThreadWin32, thread);
    return NULL;
  }
}

void sys_thread_yield (void)
{
  Sleep(0);
}

void sys_system_thread_wait (GRealThread *thread)
{
  GThreadWin32 *wt = (GThreadWin32 *) thread;

  win32_check_for_error (WAIT_FAILED != WaitForSingleObject (wt->handle, INFINITE));
}

#define EXCEPTION_SET_THREAD_NAME ((DWORD) 0x406D1388)

#ifndef _MSC_VER
static void *SetThreadName_VEH_handle = NULL;

static LONG __stdcall SetThreadName_VEH (PEXCEPTION_POINTERS ExceptionInfo)
{
  if (ExceptionInfo->ExceptionRecord != NULL &&
      ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SET_THREAD_NAME)
    return EXCEPTION_CONTINUE_EXECUTION;

  return EXCEPTION_CONTINUE_SEARCH;
}
#endif

typedef struct _THREADNAME_INFO
{
  DWORD  dwType;	/* must be 0x1000 */
  LPCSTR szName;	/* pointer to name (in user addr space) */
  DWORD  dwThreadID;	/* thread ID (-1=caller thread) */
  DWORD  dwFlags;	/* reserved for future use, must be zero */
} THREADNAME_INFO;

static void SetThreadName (DWORD  dwThreadID, LPCSTR szThreadName)
{
  THREADNAME_INFO info;
  DWORD infosize;

  info.dwType = 0x1000;
  info.szName = szThreadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  infosize = sizeof (info) / sizeof (DWORD);

#ifdef _MSC_VER
  __try
  {
    RaiseException (EXCEPTION_SET_THREAD_NAME, 0, infosize,
        (const ULONSYS_PTR *) &info);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
  }
#else
  /* Without a debugger we *must* have an exception handler,
   * otherwise raising an exception will crash the process.
   */
  if ((!IsDebuggerPresent ()) && (SetThreadName_VEH_handle == NULL))
    return;

  RaiseException (EXCEPTION_SET_THREAD_NAME, 0, infosize, (const ULONSYS_PTR *) &info);
#endif
}

typedef HRESULT (WINAPI *pSetThreadDescription) (HANDLE hThread, PCWSTR lpThreadDescription);
static pSetThreadDescription SetThreadDescriptionFunc = NULL;
HMODULE kernel32_module = NULL;

static SysBool sys_thread_win32_load_library (void)
{
  /* FIXME: Add support for UWP app */
#if !defined(SYS_WINAPI_ONLY_APP)
  static gsize _init_once = 0;
  if (sys_once_init_enter (&_init_once))
  {
    kernel32_module = LoadLibraryW (L"kernel32.dll");
    if (kernel32_module)
    {
      SetThreadDescriptionFunc =
        (pSetThreadDescription) GetProcAddress (kernel32_module,
            "SetThreadDescription");
      if (!SetThreadDescriptionFunc)
        FreeLibrary (kernel32_module);
    }
    sys_once_init_leave (&_init_once, 1);
  }
#endif

  return !!SetThreadDescriptionFunc;
}

static SysBool sys_thread_win32_set_thread_desc (const SysChar *name)
{
  HRESULT hr;
  wchar_t *namew;

  if (!sys_thread_win32_load_library () || !name)
    return FALSE;

  namew = sys_utf8_to_utf16 (name, -1, NULL, NULL, NULL);
  if (!namew)
    return FALSE;

  hr = SetThreadDescriptionFunc (GetCurrentThread (), namew);

  sys_free (namew);
  return SUCCEEDED (hr);
}

void sys_system_thread_set_name (const SysChar *name)
{
  if (!sys_thread_win32_set_thread_desc (name))
    SetThreadName ((DWORD) -1, name);
}

/* {{{1 Epilogue */

void sys_thread_win32_init (void)
{
  InitializeCriticalSection (&sys_private_lock);

#ifndef _MSC_VER
  SetThreadName_VEH_handle = AddVectoredExceptionHandler (1, &SetThreadName_VEH);
  if (SetThreadName_VEH_handle == NULL)
  {
    /* This is bad, but what can we do? */
  }
#endif
}

void sys_thread_win32_thread_detach (void)
{
  SysBool dtors_called;

  do
  {
    GPrivateDestructor *dtor;

    /* We go by the POSIX book on this one.
     *      * If we call a destructor then there is a chance that some new
     * TLS variables got set by code called in that destructor.
     *      * Loop until nothing is left.
     */
    dtors_called = FALSE;

    for (dtor = sys_atomic_pointer_get (&sys_private_destructors); dtor; dtor = dtor->next)
    {
      SysPointer value;

      value = TlsGetValue (dtor->index);
      if (value != NULL && dtor->notify != NULL)
      {
        /* POSIX says to clear this before the call */
        TlsSetValue (dtor->index, NULL);
        dtor->notify (value);
        dtors_called = TRUE;
      }
    }
  }
  while (dtors_called);
}

void sys_thread_win32_process_detach (void)
{
#ifndef _MSC_VER
  if (SetThreadName_VEH_handle != NULL)
  {
    RemoveVectoredExceptionHandler (SetThreadName_VEH_handle);
    SetThreadName_VEH_handle = NULL;
  }
#endif
}

