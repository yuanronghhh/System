#include <System/Platform/Common/SysThreadPrivate.h>
#include "SysThreadWin32.h"

typedef struct _SysWin32Thread SysWin32Thread;

struct _SysWin32Thread {
  SysRealThread parent;
  SysThreadFunc proxy;
  HANDLE      handle;
};

static CRITICAL_SECTION private_lock;
static void *SetThreadName_VEH_handle = NULL;
#define EXCEPTION_SET_THREAD_NAME ((DWORD) 0x406D1388)

static LONG __stdcall SetThreadName_VEH(PEXCEPTION_POINTERS ExceptionInfo) {
  if (ExceptionInfo->ExceptionRecord != NULL &&
    ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SET_THREAD_NAME)
    return EXCEPTION_CONTINUE_EXECUTION;

  return EXCEPTION_CONTINUE_SEARCH;
}

void sys_real_private_set(SysPrivate *key, SysPointer value) {
  DWORD rkey = POINTER_TO_UINT(key->p);
  if (key->p == NULL) {
    EnterCriticalSection(&private_lock);

    rkey = TlsAlloc();
    if (rkey == 0) {
      DWORD impl2 = TlsAlloc();
      TlsFree(rkey);
      rkey = impl2;
    }

    if (rkey == TLS_OUT_OF_INDEXES || rkey == 0) {
      sys_abort_N("%s", "win32 private_set TlsAlloc failed.");
    }

    if (!sys_atomic_ptr_cmpxchg(&key->p, NULL, UINT_TO_POINTER(rkey))) {
      sys_abort_N("%s", "win32 private_set new key failed.");
    }

    LeaveCriticalSection(&private_lock);
  }

  TlsSetValue(rkey, value);
}

SysPointer sys_real_private_get(SysPrivate *key) {
  DWORD rkey = POINTER_TO_UINT(key->p);

  return TlsGetValue(rkey);
}

void sys_real_thread_detach(void) {
  if (SetThreadName_VEH_handle != NULL) {
    RemoveVectoredExceptionHandler (SetThreadName_VEH_handle);
    SetThreadName_VEH_handle = NULL;
  }
}

void sys_real_thread_init(void) {
  InitializeCriticalSection(&private_lock);
  SetThreadName_VEH_handle = AddVectoredExceptionHandler(1, &SetThreadName_VEH);
  if (SetThreadName_VEH_handle == NULL) {
  }
}

static SysUInt WINAPI sys_inner_proxy(SysPointer data) {
  SysWin32Thread *self = data;

  self->proxy(self);
  sys_real_thread_exit();

  return 0;
}

void sys_real_thread_exit(void) {
  _endthreadex(0);
}

void sys_real_thread_wait(SysRealThread *thread) {
  SysWin32Thread *real = (SysWin32Thread *)thread;

  sys_assert(WAIT_FAILED != WaitForSingleObject(real->handle, INFINITE));
}

void sys_real_thread_free(SysRealThread *thread) {
  SysWin32Thread *real = (SysWin32Thread *)thread;

  sys_assert(CloseHandle(real->handle));
  sys_free_N(real);
}

SysRealThread *sys_real_thread_new(SysThreadFunc proxy, SysError **error) {
  SysWin32Thread *win32 = sys_new0_N(SysWin32Thread, 1);
  SysRealThread *real = (SysRealThread *)win32;
  SysUInt addr;
  SysInt prio;

  win32->proxy = proxy;
  win32->handle = (HANDLE)_beginthreadex(NULL, 0, sys_inner_proxy, win32, 0, &addr);
  if (win32->handle == NULL) {
    sys_assert(win32->handle);
    return NULL;
  }

  HANDLE current_thread = GetCurrentThread();
  prio = GetThreadPriority(current_thread);

  if (SetThreadPriority(win32->handle, prio) == 0) {
    sys_error_set_N(error, "SetThreadPriority failed");
    goto fail;
  }

  if (ResumeThread(win32->handle) == -1) {
    sys_error_set_N(error, "ResumeThread failed");
    goto fail;
  }

  sys_ref_count_init(real);

  return (SysRealThread *)win32;
fail:
  sys_free_N(win32);
  return NULL;
}

/* {{{1 SysMutex */
void sys_mutex_init(SysMutex *mutex) {
  InitializeSRWLock((SysPointer)mutex);
}

void sys_mutex_clear(SysMutex *mutex) {
}

void sys_mutex_lock(SysMutex *mutex) {
  AcquireSRWLockExclusive((SysPointer)mutex);
}

SysBool sys_mutex_trylock(SysMutex *mutex) {
  return TryAcquireSRWLockExclusive((SysPointer)mutex);
}

void sys_mutex_unlock(SysMutex *mutex) {
  ReleaseSRWLockExclusive((SysPointer)mutex);
}

static CRITICAL_SECTION * sys_rec_mutex_impl_new(void) {
  CRITICAL_SECTION *cs;

  cs = sys_slice_new(CRITICAL_SECTION);
  InitializeCriticalSection(cs);

  return cs;
}

static void sys_rec_mutex_impl_free(CRITICAL_SECTION *cs) {
  DeleteCriticalSection(cs);
  sys_slice_free(CRITICAL_SECTION, cs);
}

static CRITICAL_SECTION *sys_rec_mutex_get_impl(SysRecMutex *mutex) {
  CRITICAL_SECTION *impl = mutex->p;

  if (mutex->p == NULL) {
    impl = sys_rec_mutex_impl_new();
    if (InterlockedCompareExchangePointer(&mutex->p, impl, NULL) != NULL)
      sys_rec_mutex_impl_free(impl);
    impl = mutex->p;
  }

  return impl;
}

void sys_rec_mutex_init(SysRecMutex *mutex) {
  mutex->p = sys_rec_mutex_impl_new();
}

void sys_rec_mutex_clear(SysRecMutex *mutex) {
  sys_rec_mutex_impl_free(mutex->p);
}

void sys_rec_mutex_lock(SysRecMutex *mutex) {
  EnterCriticalSection(sys_rec_mutex_get_impl(mutex));
}

void sys_rec_mutex_unlock(SysRecMutex *mutex) {
  LeaveCriticalSection(mutex->p);
}

SysBool sys_rec_mutex_trylock(SysRecMutex *mutex) {
  return TryEnterCriticalSection(sys_rec_mutex_get_impl(mutex));
}
