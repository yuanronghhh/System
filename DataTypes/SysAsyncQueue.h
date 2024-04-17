#ifndef __SYS_ASYNCQUEUE_H__
#define __SYS_ASYNCQUEUE_H__

#include <System/DataTypes/SysQueue.h>

SYS_BEGIN_DECLS

typedef struct _SysAsyncQueue SysAsyncQueue;

SysAsyncQueue *sys_async_queue_new                  (void);
SysAsyncQueue *sys_async_queue_new_full             (SysDestroyFunc item_free_func);
void         sys_async_queue_lock                 (SysAsyncQueue      *queue);
void         sys_async_queue_unlock               (SysAsyncQueue      *queue);
SysAsyncQueue *sys_async_queue_ref                  (SysAsyncQueue      *queue);
void         sys_async_queue_unref                (SysAsyncQueue      *queue);

void         sys_async_queue_push                 (SysAsyncQueue      *queue,
                                                 SysPointer          data);
void         sys_async_queue_push_unlocked        (SysAsyncQueue      *queue,
                                                 SysPointer          data);
SysPointer     sys_async_queue_pop                  (SysAsyncQueue      *queue);
SysPointer     sys_async_queue_pop_unlocked         (SysAsyncQueue      *queue);
SysPointer     sys_async_queue_try_pop              (SysAsyncQueue      *queue);
SysPointer     sys_async_queue_try_pop_unlocked     (SysAsyncQueue      *queue);
SysPointer     sys_async_queue_timeout_pop          (SysAsyncQueue      *queue,
                                                 SysUInt64           timeout);
SysPointer     sys_async_queue_timeout_pop_unlocked (SysAsyncQueue      *queue,
                                                 SysUInt64           timeout);
SysInt         sys_async_queue_length               (SysAsyncQueue      *queue);
SysInt         sys_async_queue_length_unlocked      (SysAsyncQueue      *queue);

SysBool     sys_async_queue_remove               (SysAsyncQueue      *queue,
                                                 SysPointer          item);
SysBool     sys_async_queue_remove_unlocked      (SysAsyncQueue      *queue,
                                                 SysPointer          item);
void         sys_async_queue_push_front           (SysAsyncQueue      *queue,
                                                 SysPointer          item);
void         sys_async_queue_push_front_unlocked  (SysAsyncQueue      *queue,
                                                 SysPointer          item);

SYS_END_DECLS

#endif /* __SYS_ASYNCQUEUE_H__ */

