#include <System/Platform/Common/SysThreadPrivate.h>
#include <System/Platform/Win32/SysThreadWin32.h>
#include <System/Utils/SysStr.h>


static void
sys_thread_abort (SysInt         status,
                const SysChar *function)
{
  sys_abort_N ("GLib (gthread-win32.c): Unexpected error from C library during '%s': %s.  Aborting.\n",
    sys_strerror(status), function);
}

/* Starting with Vista and Windows 2008, we have access to the
 * CONDITION_VARIABLE and SRWLock primitives on Windows, which are
 * pretty reasonable approximations of the primitives specified in
 * POSIX 2001 (pthread_cond_t and pthread_mutex_t respectively).
 *
 * Both of these types are structs containing a single pointer.  That
 * pointer is used as an atomic bitfield to support user-space mutexes
 * that only get the kernel involved in cases of contention (similar
 * to how futex()-based mutexes work on Linux).  The biggest advantage
 * of these new types is that they can be statically initialised to
 * zero.  That means that they are completely ABI compatible with our
 * SysMutex and SysCond APIs.
 */

/* {{{1 SysMutex */
void
sys_mutex_init (SysMutex *mutex)
{
  InitializeSRWLock ((SysPointer) mutex);
}

void
sys_mutex_clear (SysMutex *mutex)
{
}

void
sys_mutex_lock (SysMutex *mutex)
{
  AcquireSRWLockExclusive ((SysPointer) mutex);
}

SysBool
sys_mutex_trylock (SysMutex *mutex)
{
  return TryAcquireSRWLockExclusive ((SysPointer) mutex);
}

void
sys_mutex_unlock (SysMutex *mutex)
{
  ReleaseSRWLockExclusive ((SysPointer) mutex);
}

/* {{{1 SysRecMutex */

static CRITICAL_SECTION *
sys_rec_mutex_impl_new (void)
{
  CRITICAL_SECTION *cs;

  cs = sys_slice_new (CRITICAL_SECTION);
  InitializeCriticalSection (cs);

  return cs;
}

static void
sys_rec_mutex_impl_free (CRITICAL_SECTION *cs)
{
  DeleteCriticalSection (cs);
  sys_slice_free (CRITICAL_SECTION, cs);
}

static CRITICAL_SECTION *
sys_rec_mutex_get_impl (SysRecMutex *mutex)
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

void
sys_rec_mutex_init (SysRecMutex *mutex)
{
  mutex->p = sys_rec_mutex_impl_new ();
}

void
sys_rec_mutex_clear (SysRecMutex *mutex)
{
  sys_rec_mutex_impl_free (mutex->p);
}

void
sys_rec_mutex_lock (SysRecMutex *mutex)
{
  EnterCriticalSection (sys_rec_mutex_get_impl (mutex));
}

void
sys_rec_mutex_unlock (SysRecMutex *mutex)
{
  LeaveCriticalSection (mutex->p);
}

SysBool
sys_rec_mutex_trylock (SysRecMutex *mutex)
{
  return TryEnterCriticalSection (sys_rec_mutex_get_impl (mutex));
}

/* {{{1 SysRWLock */

void
sys_rw_lock_init (SysRWLock *lock)
{
  InitializeSRWLock ((SysPointer) lock);
}

void
sys_rw_lock_clear (SysRWLock *lock)
{
}

void
sys_rw_lock_writer_lock (SysRWLock *lock)
{
  AcquireSRWLockExclusive ((SysPointer) lock);
}

SysBool
sys_rw_lock_writer_trylock (SysRWLock *lock)
{
  return TryAcquireSRWLockExclusive ((SysPointer) lock);
}

void
sys_rw_lock_writer_unlock (SysRWLock *lock)
{
  ReleaseSRWLockExclusive ((SysPointer) lock);
}

void
sys_rw_lock_reader_lock (SysRWLock *lock)
{
  AcquireSRWLockShared ((SysPointer) lock);
}

SysBool
sys_rw_lock_reader_trylock (SysRWLock *lock)
{
  return TryAcquireSRWLockShared ((SysPointer) lock);
}

void
sys_rw_lock_reader_unlock (SysRWLock *lock)
{
  ReleaseSRWLockShared ((SysPointer) lock);
}

/* {{{1 SysCond */
void
sys_cond_init (SysCond *cond)
{
  InitializeConditionVariable ((SysPointer) cond);
}

void
sys_cond_clear (SysCond *cond)
{
}

void
sys_cond_signal (SysCond *cond)
{
  WakeConditionVariable ((SysPointer) cond);
}

void
sys_cond_broadcast (SysCond *cond)
{
  WakeAllConditionVariable ((SysPointer) cond);
}

void
sys_cond_wait (SysCond  *cond,
             SysMutex *entered_mutex)
{
  SleepConditionVariableSRW ((SysPointer) cond, (SysPointer) entered_mutex, INFINITE, 0);
}

SysBool
sys_cond_wait_until (SysCond  *cond,
                   SysMutex *entered_mutex,
                   SysInt64  end_time)
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
      else if SYS_UNLIKELY (span > INT64_CONSTANT (1000) * (DWORD) INFINITE)
        span_millis = INFINITE;
      else
        /* Round up so we don't time out too early */
        span_millis = (DWORD)(span + 1000 - 1) / 1000;

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

/* {{{1 SysPrivate */

typedef struct _SysPrivateDestructor SysPrivateDestructor;

struct _SysPrivateDestructor
{
  DWORD               index;
  SysDestroyFunc      notify;
  SysPrivateDestructor *next;
};

static SysPrivateDestructor *sys_private_destructors;  /* (atomic) prepend-only */
static CRITICAL_SECTION sys_private_lock;
static SysPrivateDestructor priv_destructor;

static DWORD
sys_private_get_impl (SysPrivate *key)
{
  DWORD impl = (DWORD) POINTER_TO_UINT(key->p);

  if SYS_UNLIKELY (impl == 0)
    {
      EnterCriticalSection (&sys_private_lock);
      impl = (DWORD)POINTER_TO_UINT(key->p);
      if (impl == 0)
        {
          SysPrivateDestructor *destructor;

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
              destructor = &priv_destructor; // malloc (sizeof (SysPrivateDestructor));
              if SYS_UNLIKELY(destructor == NULL) {

                sys_thread_abort (errno, "malloc");
              }
              destructor->index = impl;
              destructor->notify = key->notify;
              destructor->next = sys_atomic_pointer_get (&sys_private_destructors);

              /* We need to do an atomic store due to the unlocked
               * access to the destructor list from the thread exit
               * function.
               *
               * It can double as a sanity check...
               */
              if (!sys_atomic_pointer_cmpxchg ((volatile SysPointer *)(&sys_private_destructors),
                                                          destructor->next,
                                                          destructor))
                sys_thread_abort (0, "sys_private_get_impl(1)");
            }

          /* Ditto, due to the unlocked access on the fast path */
          if (!sys_atomic_pointer_cmpxchg (&key->p, NULL, UINT_TO_POINTER (impl)))
            sys_thread_abort (0, "sys_private_get_impl(2)");
        }
      LeaveCriticalSection (&sys_private_lock);
    }

  return impl;
}

SysPointer
sys_private_get (SysPrivate *key)
{
  return TlsGetValue (sys_private_get_impl (key));
}

void
sys_private_set (SysPrivate *key,
               SysPointer  value)
{
  TlsSetValue(sys_private_get_impl(key), value);
}

void
sys_private_replace (SysPrivate *key,
                   SysPointer  value)
{
  DWORD impl = sys_private_get_impl (key);
  SysPointer old;

  old = TlsGetValue (impl);
  TlsSetValue (impl, value);
  if (old && key->notify)
    key->notify (old);
}

/* {{{1 SysThread */

#define win32_check_for_error(what) SYS_STMT_START{			\
  if (!(what))								\
    sys_error_N ("error %s during %s",	GetLastError (), #what);		\
  }SYS_STMT_END

typedef BOOL (__stdcall *GTryEnterCriticalSectionFunc) (CRITICAL_SECTION *);

typedef struct
{
  SysRealThread thread;

  SysThreadFunc proxy;
  HANDLE      handle;
} SysThreadWin32;

void
sys_system_thread_free (SysRealThread *thread)
{
  SysThreadWin32 *wt = (SysThreadWin32 *) thread;

  win32_check_for_error (CloseHandle (wt->handle));
  sys_slice_free (SysThreadWin32, wt);
}

void
sys_system_thread_exit (void)
{
  /* In static compilation, DllMain doesn't exist and so DLL_THREAD_DETACH
   * case is never called and thread destroy notifications are not triggered.
   * To ensure that notifications are correctly triggered in static
   * compilation mode, we call directly the "detach" function here right
   * before terminating the thread.
   * As all win32 threads initialized through the glib API are run through
   * the same proxy function sys_thread_win32_proxy() which calls systematically
   * sys_system_thread_exit() when finishing, we obtain the same behavior as
   * with dynamic compilation.
   *
   * WARNING: unfortunately this mechanism cannot work with threads created
   * directly from the Windows API using CreateThread() or _beSysInthread/ex().
   * It only works with threads created by using the glib API with
   * sys_system_thread_new(). If users need absolutely to use a thread NOT
   * created with glib API under Windows and in static compilation mode, they
   * should not use glib functions within their thread or they may encounter
   * memory leaks when the thread finishes.
   */
  sys_system_thread_detach();

  _endthreadex (0);
}

static SysUInt __stdcall
sys_thread_win32_proxy (SysPointer data)
{
  SysThreadWin32 *self = data;

  self->proxy (self);

  sys_system_thread_exit ();

  return 0;
}

SysRealThread *
sys_system_thread_new (SysThreadFunc proxy,
                     SysSize stack_size,
                     const SysChar *name,
                     SysThreadFunc func,
                     SysPointer data,
                     SysError **error)
{
  SysThreadWin32 *thread;
  SysRealThread *base_thread;
  SysUInt ignore;
  const SysChar *message = NULL;
  SysInt thread_prio;

  thread = sys_slice_new0 (SysThreadWin32);
  thread->proxy = proxy;
  thread->handle = (HANDLE) NULL;
  base_thread = (SysRealThread*)thread;
  base_thread->ref_count = 2;
  base_thread->ours = true;
  base_thread->thread.joinable = true;
  base_thread->thread.func = func;
  base_thread->thread.data = data;
  base_thread->name = sys_strdup (name);

  thread->handle = (HANDLE) _beginthreadex(NULL, (SysUInt)stack_size, sys_thread_win32_proxy, thread,
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
   *
   * On Windows, by default all new threads are created with NORMAL thread
   * priority.
   */
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

  if (ResumeThread (thread->handle) == (DWORD) -1)
    {
      message = "Error resuming new thread";
      goto error;
    }
  UNUSED(message);

  return (SysRealThread *) thread;

error:
  {
    if (thread->handle)
      CloseHandle (thread->handle);
    sys_slice_free (SysThreadWin32, thread);
    return NULL;
  }
}

void
sys_thread_yield (void)
{
  Sleep(0);
}

void
sys_system_thread_wait (SysRealThread *thread)
{
  SysThreadWin32 *wt = (SysThreadWin32 *) thread;

  win32_check_for_error (WAIT_FAILED != WaitForSingleObject (wt->handle, INFINITE));
}

#define EXCEPTION_SET_THREAD_NAME ((DWORD) 0x406D1388)

#ifndef _MSC_VER
static void *SetThreadName_VEH_handle = NULL;

static LONG __stdcall
SetThreadName_VEH (PEXCEPTION_POINTERS ExceptionInfo)
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

static void
SetThreadName (DWORD  dwThreadID,
               LPCSTR szThreadName)
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
                       (const ULONG_PTR *) &info);
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

typedef HRESULT (WINAPI *pSetThreadDescription) (HANDLE hThread,
                                                 PCWSTR lpThreadDescription);
static pSetThreadDescription SetThreadDescriptionFunc = NULL;
static HMODULE kernel32_module = NULL;

static SysBool
sys_thread_win32_load_library (void)
{
  /* FIXME: Add support for UWP app */
#if !defined(SYS_WINAPI_ONLY_APP)
  static SysSize _init_once = 0;
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

static SysBool
sys_thread_win32_set_thread_desc (const SysChar *name)
{
  HRESULT hr;
  wchar_t *namew;

  if (!sys_thread_win32_load_library () || !name)
    return false;

  namew = sys_mbyte_to_wchar(name, NULL);
  if (!namew)
    return false;

  hr = SetThreadDescriptionFunc (GetCurrentThread (), namew);

  sys_free (namew);
  return SUCCEEDED (hr);
}

void
sys_system_thread_set_name (const SysChar *name)
{
  /* Prefer SetThreadDescription over exception based way if available,
   * since thread description set by SetThreadDescription will be preserved
   * in dump file */
  if (!sys_thread_win32_set_thread_desc (name))
    SetThreadName ((DWORD) -1, name);
}

/* {{{1 Epilogue */

void
sys_system_thread_init (void)
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

void
sys_system_thread_detach (void)
{
  SysBool dtors_called;

  do
    {
      SysPrivateDestructor *dtor;

      /* We go by the POSIX book on this one.
       *
       * If we call a destructor then there is a chance that some new
       * TLS variables got set by code called in that destructor.
       *
       * Loop until nothing is left.
       */
      dtors_called = false;
      for (dtor = sys_atomic_pointer_get (&sys_private_destructors); dtor; dtor = dtor->next)
        {
          SysPointer value;
          
          value = TlsGetValue (dtor->index);
          if (value != NULL && dtor->notify != NULL)
            {
              /* POSIX says to clear this before the call */
              TlsSetValue (dtor->index, NULL);
              dtor->notify (value);
              dtors_called = true;
            }
        }
    }
  while (dtors_called);
}

void sys_thread_win32_process_detach (void) {
#ifndef _MSC_VER
  if (SetThreadName_VEH_handle != NULL)
    {
      RemoveVectoredExceptionHandler (SetThreadName_VEH_handle);
      SetThreadName_VEH_handle = NULL;
    }
#endif
}

/* vim:set foldmethod=marker: */
