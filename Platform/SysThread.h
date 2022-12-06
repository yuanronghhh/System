#ifndef __SYS_THREAD_H__
#define __SYS_THREAD_H__

#include <Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysMutex SysMutex;
typedef SysPointer (*SysThreadFunc) (SysPointer user_data);

typedef struct _SysThread SysThread;
typedef struct _SysPrivate SysPrivate;

struct _SysThread {
  SysPointer data;
  SysThreadFunc func;
};

struct _SysMutex {
  SysPointer lock;
};

struct _SysPrivate {
  SysPointer p;
  SysDestroyFunc dest_func;
};

SYS_API SysPointer sys_private_get(SysPrivate *key);
SYS_API void sys_private_set(SysPrivate *key, SysPointer value);


/* Mutex */
SYS_API void sys_mutex_free(SysMutex *mutex);
SYS_API SysMutex *sys_mutex_new();
SYS_API void sys_mutex_lock(SysMutex *mutex);
SYS_API bool sys_mutex_trylock(SysMutex *mutex);
SYS_API void sys_mutex_unlock(SysMutex *mutex);

SYS_API void sys_thread_init(void);
SYS_API void sys_thread_detach(void);
SYS_API void sys_thread_exit(void);
SYS_API SysThread *sys_thread_new(SysThreadFunc func, SysPointer user_data);
SYS_API SysPointer sys_thread_join(SysThread *thread);
SYS_API SysThread* sys_thread_self(void);
SYS_API void sys_thread_unref(SysThread *thread);

SYS_END_DECLS

#endif
