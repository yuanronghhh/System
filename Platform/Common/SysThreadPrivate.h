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

SYS_END_DECLS

#endif
