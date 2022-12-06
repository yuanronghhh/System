#ifndef __SYS_THREAD_PRIVATE_H__
#define __SYS_THREAD_PRIVATE_H__

#include <Platform/SysThread.h>

SYS_BEGIN_DECLS

typedef struct _SysRealThread SysRealThread;
typedef struct _SysRealMutex SysRealMutex;

struct _SysRealThread {
  SysThread thread;
  SysPointer retval;
  SysRef ref_count;
};

struct _SysRealMutex {
  SysMutex mutex;
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

/* Mutex */
SysRealMutex *sys_real_mutex_new();
void sys_real_mutex_lock(SysRealMutex *mutex);
bool sys_real_mutex_trylock(SysRealMutex *mutex);
void sys_real_mutex_unlock(SysRealMutex *mutex);
void sys_real_mutex_free(SysRealMutex *mutex);

SYS_END_DECLS

#endif
