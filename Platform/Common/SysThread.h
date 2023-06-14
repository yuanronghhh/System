#ifndef __SYS_THREAD_H__
#define __SYS_THREAD_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef union _SysMutex SysMutex;
typedef struct _SysRecMutex SysRecMutex;
typedef struct _SysRWLock SysRWLock;
typedef struct _SysCond SysCond;
typedef SysPointer (*SysThreadFunc) (SysPointer user_data);

typedef struct _SysThread SysThread;
typedef struct _SysPrivate SysPrivate;

struct _SysThread {
  SysPointer data;
  SysThreadFunc func;
};

union _SysMutex {
  SysPointer p;
  SysUInt i[2];
};

struct _SysRecMutex {
  SysPointer p;
  SysUInt i[2];
};

struct _SysRWLock {
  SysPointer p;
  SysUInt i[2];
};

struct _SysCond {
  SysPointer p;
  SysUInt i[2];
};

#define SYS_PRIVATE_INIT(notify) { NULL, (notify) }
struct _SysPrivate {
  SysPointer p;
  SysDestroyFunc dest_func;
};

SYS_API SysPointer sys_private_get(SysPrivate *key);
SYS_API void sys_private_set(SysPrivate *key, SysPointer value);

/* Mutex */
SYS_API void sys_mutex_clear(SysMutex *mutex);
SYS_API void sys_mutex_init(SysMutex *mutex);
SYS_API void sys_mutex_lock(SysMutex *mutex);
SYS_API bool sys_mutex_trylock(SysMutex *mutex);
SYS_API void sys_mutex_unlock(SysMutex *mutex);

SYS_API void sys_rec_mutex_init(SysRecMutex *mutex);
SYS_API void sys_rec_mutex_clear(SysRecMutex *mutex);
SYS_API void sys_rec_mutex_lock(SysRecMutex *mutex);
SYS_API void sys_rec_mutex_unlock(SysRecMutex *mutex);
SYS_API SysBool sys_rec_mutex_trylock(SysRecMutex *mutex);

SYS_API void sys_thread_init(void);
SYS_API void sys_thread_detach(void);
SYS_API void sys_thread_exit(void);
SYS_API SysThread *sys_thread_new(SysThreadFunc func, SysPointer user_data);
SYS_API SysPointer sys_thread_join(SysThread *thread);
SYS_API SysThread* sys_thread_self(void);
SYS_API void sys_thread_unref(SysThread *thread);

SYS_END_DECLS

#endif
