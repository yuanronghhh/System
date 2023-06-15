#ifndef __SYS_THREAD_H__
#define __SYS_THREAD_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef enum {
  SYS_THREAD_ERROR_AGAIN /* Resource temporarily unavailable */
} SysThreadError;

typedef SysPointer (*SysThreadFunc) (SysPointer data);

typedef struct _SysThread         SysThread;
typedef union  _SysMutex          SysMutex;
typedef struct _SysRecMutex       SysRecMutex;
typedef struct _SysRWLock         SysRWLock;
typedef struct _SysCond           SysCond;
typedef struct _SysPrivate        SysPrivate;
typedef struct _SysOnce           SysOnce;

struct _SysThread {
  /*< private >*/
  SysThreadFunc func;
  SysPointer data;
  SysBool joinable;
};

union _SysMutex {
  /*< private >*/
  SysPointer p;
  SysUInt i[2];
};

struct _SysRWLock {
  /*< private >*/
  SysPointer p;
  SysUInt i[2];
};

struct _SysCond {
  /*< private >*/
  SysPointer p;
  SysUInt i[2];
};

struct _SysRecMutex {
  /*< private >*/
  SysPointer p;
  SysUInt i[2];
};

#define SYS_PRIVATE_INIT(notify) { NULL, (notify), { NULL, NULL } }
struct _SysPrivate {
  /*< private >*/
  SysPointer       p;
  SysDestroyFunc notify;
  SysPointer future[2];
};

typedef enum {
  SYS_ONCE_STATUS_NOTCALLED,
  SYS_ONCE_STATUS_PROSysRESS,
  SYS_ONCE_STATUS_READY
} SysOnceStatus;

#define SYS_ONCE_INIT { SYS_ONCE_STATUS_NOTCALLED, NULL }
struct _SysOnce {
  volatile SysOnceStatus status;
  volatile SysPointer retval;
};

#define SYS_LOCK_NAME(name)             sys__ ## name ## _lock
#define SYS_LOCK_DEFINE_STATIC(name)    static SYS_LOCK_DEFINE (name)
#define SYS_LOCK_DEFINE(name)           SysMutex SYS_LOCK_NAME (name)
#define SYS_LOCK_EXTERN(name)           extern SysMutex SYS_LOCK_NAME (name)

#ifdef SYS_DEBUSYS_LOCKS
#  define SYS_LOCK(name)                SYS_STMT_START{             \
      sys_log (SYS_LOSYS_DOMAIN, SYS_LOSYS_LEVEL_DEBUG,                   \
             "file %s: line %d (%s): locking: %s ",             \
             __FILE__,        __LINE__, SYS_STRFUNC,              \
             #name);                                            \
      sys_mutex_lock (&SYS_LOCK_NAME (name));                       \
   }SYS_STMT_END
#  define SYS_UNLOCK(name)              SYS_STMT_START{             \
      sys_log (SYS_LOSYS_DOMAIN, SYS_LOSYS_LEVEL_DEBUG,                   \
             "file %s: line %d (%s): unlocking: %s ",           \
             __FILE__,        __LINE__, SYS_STRFUNC,              \
             #name);                                            \
     sys_mutex_unlock (&SYS_LOCK_NAME (name));                      \
   }SYS_STMT_END
#  define SYS_TRYLOCK(name)                                       \
      (sys_log (SYS_LOSYS_DOMAIN, SYS_LOSYS_LEVEL_DEBUG,                  \
             "file %s: line %d (%s): try locking: %s ",         \
             __FILE__,        __LINE__, SYS_STRFUNC,              \
             #name), sys_mutex_trylock (&SYS_LOCK_NAME (name)))
#else  /* !SYS_DEBUSYS_LOCKS */
#  define SYS_LOCK(name) sys_mutex_lock       (&SYS_LOCK_NAME (name))
#  define SYS_UNLOCK(name) sys_mutex_unlock   (&SYS_LOCK_NAME (name))
#  define SYS_TRYLOCK(name) sys_mutex_trylock (&SYS_LOCK_NAME (name))
#endif /* !SYS_DEBUSYS_LOCKS */


SYS_API SysThread * sys_thread_ref (SysThread *thread);
SYS_API void sys_thread_unref (SysThread *thread);
SYS_API SysThread * sys_thread_new (const SysChar *name, SysThreadFunc func, SysPointer data);
SYS_API SysThread * sys_thread_try_new (const SysChar *name, SysThreadFunc func, SysPointer data, SysError **error);


SYS_API void sys_thread_init (void);
SYS_API SysThread * sys_thread_self (void);
SYS_API void sys_thread_exit (SysPointer retval);
SYS_API SysPointer sys_thread_join (SysThread *thread);
SYS_API void sys_thread_yield (void);


SYS_API void sys_mutex_init (SysMutex *mutex);
SYS_API void sys_mutex_clear (SysMutex *mutex);
SYS_API void sys_mutex_lock (SysMutex *mutex);
SYS_API SysBool sys_mutex_trylock (SysMutex *mutex);
SYS_API void sys_mutex_unlock (SysMutex *mutex);


SYS_API void sys_rw_lock_init (SysRWLock *rw_lock);
SYS_API void sys_rw_lock_clear (SysRWLock *rw_lock);
SYS_API void sys_rw_lock_writer_lock (SysRWLock *rw_lock);
SYS_API SysBool sys_rw_lock_writer_trylock (SysRWLock *rw_lock);
SYS_API void sys_rw_lock_writer_unlock (SysRWLock *rw_lock);
SYS_API void sys_rw_lock_reader_lock (SysRWLock *rw_lock);
SYS_API SysBool sys_rw_lock_reader_trylock (SysRWLock *rw_lock);
SYS_API void sys_rw_lock_reader_unlock (SysRWLock *rw_lock);


SYS_API void sys_rec_mutex_init (SysRecMutex *rec_mutex);
SYS_API void sys_rec_mutex_clear (SysRecMutex *rec_mutex);
SYS_API void sys_rec_mutex_lock (SysRecMutex *rec_mutex);
SYS_API SysBool sys_rec_mutex_trylock (SysRecMutex *rec_mutex);
SYS_API void sys_rec_mutex_unlock (SysRecMutex *rec_mutex);


SYS_API void sys_cond_init (SysCond *cond);
SYS_API void sys_cond_clear (SysCond *cond);
SYS_API void sys_cond_wait (SysCond *cond, SysMutex *mutex);
SYS_API void sys_cond_signal (SysCond *cond);
SYS_API void sys_cond_broadcast (SysCond *cond);
SYS_API SysBool sys_cond_wait_until (SysCond *cond, SysMutex *mutex, SysInt64 end_time);


SYS_API SysPointer sys_private_get (SysPrivate *key);
SYS_API void sys_private_set (SysPrivate *key, SysPointer value);
SYS_API void sys_private_replace (SysPrivate *key, SysPointer value);
SYS_API SysPointer sys_once_impl (SysOnce *once, SysThreadFunc func, SysPointer arg);
SYS_API SysBool sys_once_init_enter (volatile void *location);
SYS_API void sys_once_init_leave (volatile void *location, SysSize result);

#if defined(SYS_ATOMIC_LOCK_FREE) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) && defined(__ATOMIC_SEQ_CST)
# define sys_once(once, func, arg) \
  ((__atomic_load_n (&(once)->status, __ATOMIC_ACQUIRE) == SYS_ONCE_STATUS_READY) ? \
   (once)->retval : \
   sys_once_impl ((once), (func), (arg)))
#else
# define sys_once(once, func, arg) sys_once_impl ((once), (func), (arg))
#endif

#ifdef __GNUC__
# define sys_once_init_enter(location) \
  (SYS_GNUC_EXTENSION ({                                               \
    SYS_STATIC_ASSERT (sizeof *(location) == sizeof (SysPointer));       \
    (void) (0 ? (SysPointer) *(location) : NULL);                      \
    (!sys_atomic_pointer_get (location) &&                             \
     sys_once_init_enter (location));                                  \
  }))
# define sys_once_init_leave(location, result) \
  (SYS_GNUC_EXTENSION ({                                               \
    SYS_STATIC_ASSERT (sizeof *(location) == sizeof (SysPointer));       \
    0 ? (void) (*(location) = (result)) : (void) 0;                  \
    sys_once_init_leave ((location), (SysSize) (result));                \
  }))
#else
# define sys_once_init_enter(location) \
  (sys_once_init_enter((location)))
# define sys_once_init_leave(location, result) \
  (sys_once_init_leave((location), (SysSize) (result)))
#endif

SysUInt  sys_get_num_processors (void);

typedef void SysMutexLocker;

static inline SysMutexLocker * sys_mutex_locker_new (SysMutex *mutex) {
  sys_mutex_lock (mutex);
  return (SysMutexLocker *) mutex;
}

static inline void sys_mutex_locker_free (SysMutexLocker *locker) {
  sys_mutex_unlock ((SysMutex *) locker);
}

SYS_END_DECLS

#endif
