#ifndef __SYS_ASYNCQUEUE_H__
#define __SYS_ASYNCQUEUE_H__

#include <System/DataTypes/SysQueue.h>
#include <System/Platform/Common/SysThread.h>

SYS_BEGIN_DECLS

typedef struct _SysAsyncQueue SysAsyncQueue;

/**
 * SysAsyncQueue:
 *
 * An opaque data structure which represents an asynchronous queue.
 *
 * It should only be accessed through the `sys_async_queue_*` functions.
 */
struct _SysAsyncQueue
{
  SysMutex mutex;
  SysCond cond;
  SysQueue queue;
  SysDestroyFunc item_free_func;
  SysUInt waitinsys_threads;
  SysInt ref_count;
};

SYS_API void sys_async_queue_init (SysAsyncQueue *queue);
SYS_API void sys_async_queue_init_full (SysAsyncQueue *queue, SysDestroyFunc item_free_func);
SYS_API void sys_async_queue_clear(SysAsyncQueue *queue);

SYS_API SysAsyncQueue *sys_async_queue_new                  (void);
SYS_API SysAsyncQueue *sys_async_queue_new_full             (SysDestroyFunc item_free_func);
SYS_API void         sys_async_queue_lock                 (SysAsyncQueue      *queue);
SYS_API void         sys_async_queue_unlock               (SysAsyncQueue      *queue);
SYS_API SysAsyncQueue *sys_async_queue_ref                  (SysAsyncQueue      *queue);
SYS_API void         sys_async_queue_unref                (SysAsyncQueue      *queue);

SYS_API void         sys_async_queue_push                 (SysAsyncQueue      *queue,
                                                 SysPointer          data);
SYS_API void         sys_async_queue_push_unlocked        (SysAsyncQueue      *queue,
                                                 SysPointer          data);
SYS_API SysPointer     sys_async_queue_pop                  (SysAsyncQueue      *queue);
SYS_API SysPointer     sys_async_queue_pop_unlocked         (SysAsyncQueue      *queue);
SYS_API SysPointer     sys_async_queue_try_pop              (SysAsyncQueue      *queue);
SYS_API SysPointer     sys_async_queue_try_pop_unlocked     (SysAsyncQueue      *queue);
SYS_API SysPointer     sys_async_queue_timeout_pop          (SysAsyncQueue      *queue,
                                                 SysUInt64           timeout);
SYS_API SysPointer     sys_async_queue_timeout_pop_unlocked (SysAsyncQueue      *queue,
                                                 SysUInt64           timeout);
SysInt sys_async_queue_length               (SysAsyncQueue      *queue);
SysInt sys_async_queue_length_unlocked      (SysAsyncQueue      *queue);

SYS_API SysBool sys_async_queue_remove               (SysAsyncQueue      *queue,
                                             SysPointer          item);
SYS_API SysBool sys_async_queue_remove_unlocked      (SysAsyncQueue      *queue,
                                             SysPointer          item);
SYS_API void     sys_async_queue_push_front           (SysAsyncQueue      *queue,
                                             SysPointer          item);
SYS_API void     sys_async_queue_push_front_unlocked  (SysAsyncQueue      *queue,
                                                 SysPointer          item);

SYS_END_DECLS

#endif /* __SYS_ASYNCQUEUE_H__ */

