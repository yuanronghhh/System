#ifndef __SYS_THREAD_PRIVATE_H__
#define __SYS_THREAD_PRIVATE_H__

#include <System/Platform/Common/SysThread.h>

SYS_BEGIN_DECLS

typedef struct _SysRealThread SysRealThread;

/* Platform-specific scheduler settings for a thread */
typedef struct {
#if SYS_OS_UNIX
  /* This is for modern Linux */
  struct sched_attr *attr;
#elif SYS_OS_WIN32
  SysInt thread_prio;
#else
  void *dummy;
#endif
} SysThreadSchedulerSettings;

struct _SysRealThread {
  SysThread thread;

  SysInt ref_count;
  SysBool ours;
  SysChar *name;
  SysPointer retval;
};

void            sys_system_thread_wait            (SysRealThread  *thread);

SysRealThread *   sys_system_thread_new             (SysThreadFunc   func,
    SysULong        stack_size,
    SysError      **error);
void            sys_system_thread_free            (SysRealThread  *thread);

void            sys_system_thread_exit            (void);
void            sys_system_thread_set_name        (const SysChar  *name);

/* gthread.c */
SysThread *       sys_thread_new_internal           (const SysChar  *name,
    SysThreadFunc   proxy,
    SysThreadFunc   func,
    SysPointer      data,
    SysSize         stack_size,
    SysError      **error);

SysPointer        sys_thread_proxy                  (SysPointer      thread);


SYS_END_DECLS

#endif
