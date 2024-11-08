#ifndef __SYS_QUEUE_H__
#define __SYS_QUEUE_H__

#include <System/DataTypes/SysList.h>

SYS_BEGIN_DECLS

#define sys_queue_foreach(queue, node)  sys_list_foreach((queue)->head, node)

typedef struct _SysQueue SysQueue;

struct _SysQueue {
  SysList *head;
  SysList *tail;
  SysUInt  length;
};

SYS_API SysQueue* sys_queue_new(void);
SYS_API void  sys_queue_free(SysQueue *queue);
SYS_API void  sys_queue_free_full(SysQueue *queue, SysDestroyFunc free_func);
SYS_API void  sys_queue_init(SysQueue *queue);
SYS_API void  sys_queue_clear(SysQueue *queue);
SYS_API SysBool sys_queue_is_empty(SysQueue *queue);
SYS_API SysUInt  sys_queue_get_length(SysQueue *queue);
SYS_API void  sys_queue_reverse(SysQueue *queue);
SYS_API SysQueue * sys_queue_copy(SysQueue *queue);
SYS_API SysList * sys_queue_find(SysQueue *queue, const SysPointer  data);
SYS_API SysList * sys_queue_find_custom(SysQueue *queue, const SysPointer  data, SysCompareFunc   func);
SYS_API void  sys_queue_push_head(SysQueue *queue, SysPointer  data);
SYS_API void sys_queue_push_nth(SysQueue* queue, SysPointer  data, SysInt n);
SYS_API void  sys_queue_push_head_link(SysQueue *queue, SysList *link);
SYS_API void  sys_queue_push_tail(SysQueue *queue, SysPointer  data);
SYS_API void  sys_queue_push_tail_link(SysQueue *queue, SysList *link);
SYS_API void  sys_queue_push_nth_link(SysQueue *queue, SysInt    n, SysList *link_);
SYS_API SysPointer  sys_queue_pop_head(SysQueue *queue);
SYS_API SysList * sys_queue_pop_head_link(SysQueue *queue);
SYS_API SysList * sys_queue_peek_head_link(SysQueue *queue);
SYS_API SysList * sys_queue_peek_tail_link(SysQueue *queue);
SYS_API SysPointer  sys_queue_pop_tail(SysQueue *queue);
SYS_API SysPointer  sys_queue_pop_nth(SysQueue *queue, SysUInt   n);
SYS_API SysList * sys_queue_pop_tail_link(SysQueue *queue);
SYS_API SysList * sys_queue_pop_nth_link(SysQueue *queue, SysUInt   n);
SYS_API SysList * sys_queue_peek_nth_link(SysQueue *queue, SysUInt   n);
SYS_API SysInt  sys_queue_link_index(SysQueue *queue, SysList *link_);
SYS_API void  sys_queue_unlink(SysQueue *queue, SysList *link_);
SYS_API void  sys_queue_delete_link(SysQueue *queue, SysList *link_);
SYS_API SysPointer  sys_queue_peek_head(SysQueue *queue);
SYS_API SysPointer  sys_queue_peek_tail(SysQueue *queue);
SYS_API SysPointer  sys_queue_peek_nth(SysQueue *queue, SysUInt   n);
SYS_API SysInt  sys_queue_index(SysQueue *queue, const SysPointer  data);
SYS_API SysBool  sys_queue_remove(SysQueue *queue, const SysPointer  data);
SYS_API SysUInt  sys_queue_remove_all(SysQueue *queue, const SysPointer  data);
SYS_API void  sys_queue_insert_before(SysQueue *queue, SysList *sibling, SysPointer  data);
SYS_API void  sys_queue_insert_after(SysQueue *queue, SysList *sibling, SysPointer  data);

SYS_END_DECLS

#endif
