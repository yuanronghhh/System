#ifndef __SYS_HQUEUE_H__
#define __SYS_HQUEUE_H__

#include <System/DataTypes/SysHList.h>

SYS_BEGIN_DECLS


#define sys_hqueue_push_head(queue, link) sys_hqueue_push_head_link(queue, link)
#define sys_hqueue_push_tail(queue, link) sys_hqueue_push_tail_link(queue, link)
#define sys_hqueue_pop_head(queue) sys_hqueue_pop_head_link(queue)
#define sys_hqueue_pop_tail(queue) sys_hqueue_pop_tail_link(queue)
#define sys_hqueue_peek_nth(queue, n) sys_hqueue_peek_nth_link(queue, n)
#define sys_hqueue_pop_nth(queue, n) sys_hqueue_pop_nth_link(queue, n)
#define sys_hqueue_peek_head(queue)  sys_hqueue_peek_head_link(queue)
#define sys_hqueue_peek_tail(queue)  sys_hqueue_peek_tail_link(queue)
#define sys_hqueue_foreach(queue, node)  sys_hlist_foreach((queue)->head, node)

typedef struct _SysHQueue SysHQueue;

struct _SysHQueue {
  SysHList *head;
  SysHList *tail;
  SysUInt  length;
};

SYS_API void  sys_hqueue_init(SysHQueue *queue);
SYS_API void  sys_hqueue_clear(SysHQueue *queue);
SYS_API SysBool sys_hqueue_is_empty(SysHQueue *queue);
SYS_API SysUInt  sys_hqueue_get_length(SysHQueue *queue);
SYS_API void  sys_hqueue_reverse(SysHQueue *queue);
SYS_API SysHList * sys_hqueue_find(SysHQueue *queue, const SysHList *new_list);
SYS_API SysHList * sys_hqueue_find_custom(SysHQueue *queue, const SysHList *new_list, SysCompareFunc   func);
SYS_API void sys_hqueue_push_nth(SysHQueue* queue, SysHList *new_list, SysInt n);
SYS_API void  sys_hqueue_push_head_link(SysHQueue *queue, SysHList *link);
SYS_API void  sys_hqueue_push_tail_link(SysHQueue *queue, SysHList *link);
SYS_API void  sys_hqueue_push_nth_link(SysHQueue *queue, SysInt    n, SysHList *link_);
SYS_API SysHList * sys_hqueue_pop_head_link(SysHQueue *queue);
SYS_API SysHList * sys_hqueue_peek_head_link(SysHQueue *queue);
SYS_API SysHList * sys_hqueue_peek_tail_link(SysHQueue *queue);
SYS_API SysHList * sys_hqueue_pop_tail_link(SysHQueue *queue);
SYS_API SysHList * sys_hqueue_pop_nth_link(SysHQueue *queue, SysUInt   n);
SYS_API SysHList * sys_hqueue_peek_nth_link(SysHQueue *queue, SysUInt   n);
SYS_API SysInt  sys_hqueue_link_index(SysHQueue *queue, SysHList *link_);
SYS_API void  sys_hqueue_unlink(SysHQueue *queue, SysHList *link_);
SYS_API SysInt  sys_hqueue_index(SysHQueue *queue, const SysHList *new_list);
SYS_API void  sys_hqueue_insert_before(SysHQueue *queue, SysHList *sibling, SysHList *new_list);
SYS_API void  sys_hqueue_insert_after(SysHQueue *queue, SysHList *sibling, SysHList *new_list);

SYS_END_DECLS

#endif
