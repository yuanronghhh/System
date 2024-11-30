#ifndef __SYS_THREAD_PRIVATE_H__
#define __SYS_THREAD_PRIVATE_H__

#include <System/Platform/Common/SysThread.h>


SYS_BEGIN_DECLS

typedef struct _SysRealThread SysRealThread;
struct  _SysRealThread
{
  SysThread thread;
  SysBool ours;
  SysChar *name;
  SysPointer retval;
};

void sys_system_thread_init(void);
void sys_system_thread_detach(void);

void            sys_system_thread_wait            (SysRealThread  *thread);

SysRealThread *sys_system_thread_new (SysThreadFunc proxy,
                                  SysSize stack_size,
                                  const SysChar *name,
                                  SysThreadFunc func,
                                  SysPointer data,
                                  SysError **error);
void            sys_system_thread_free            (SysRealThread  *thread);

void            sys_system_thread_exit            (void);
void            sys_system_thread_set_name        (const SysChar  *name);

/* gthread.c */
SysThread *sys_thread_new_internal (const SysChar *name,
                                SysThreadFunc proxy,
                                SysThreadFunc func,
                                SysPointer data,
                                SysSize stack_size,
                                SysError **error);

SysPointer        sys_thread_proxy                  (SysPointer      thread);

SysUInt           sys_thread_n_created              (void);

SysPointer        sys_private_set_alloc0            (SysPrivate       *key,
                                                 SysSize           size);

SYS_END_DECLS

#endif

