#ifndef __SYS_THREAD_PRIVATE_H__
#define __SYS_THREAD_PRIVATE_H__

#include <System/Platform/Common/SysThread.h>

SYS_BEGIN_DECLS

typedef struct _SysRealThread SysRealThread;

struct _SysRealThread {
  SysThread thread;
  SysPointer retval;
  SysRef ref_count;
};

void sys_real_private_set(SysPrivate *key, SysPointer value);
SysPointer sys_real_private_get(SysPrivate *key);

void sys_real_thread_init(void);
void sys_real_thread_detach(void);
SysRealThread *sys_real_thread_new(SysThreadFunc proxy, SysError **error);
void sys_real_thread_exit(void);
void sys_real_thread_wait(SysRealThread *thread);
void sys_real_thread_free(SysRealThread *thread);

SYS_END_DECLS

#endif
