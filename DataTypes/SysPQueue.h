#ifndef __SYS_PQUEUE_H__
#define __SYS_PQUEUE_H__

#include <System/DataTypes/SysQueue.h>
#include <System/DataTypes/SysList.h>

#define SYS_P_NODE(o) ((SysPNode *)o)

SYS_BEGIN_DECLS

typedef struct _SysQueue SysPQueue;
typedef struct _SysPNode SysPNode;

struct _SysPNode {
  SysList parent;
  SysInt prio;
};

#define sys_pqueue_foreach(queue, node) sys_queue_foreach(queue, node)
#define sys_pqueue_peek_head(queue) sys_queue_peek_head(queue)
#define sys_pnode_next(o) (SYS_P_NODE(o)->parent.next)
#define sys_pnode_prev(o) (SYS_P_NODE(o)->parent.prev)

SysPQueue *sys_pqueue_new(void);
void sys_pqueue_init(SysPQueue *queue);
void sys_pqueue_destroy(SysPQueue *queue, SysDestroyFunc free_func);
SysPNode *sys_pqueue_push_tail(SysPQueue *queue, SysInt prio, SysPointer data);
SysPNode* sys_pqueue_push_head(SysPQueue* queue, SysInt prio, SysPointer data);

void sys_pqueue_unlink(SysPQueue *queue, SysPNode *plink);
void sys_pqueue_push_tail_link(SysPQueue *queue, SysPNode *plink);
void sys_pqueue_push_head_link(SysPQueue *queue, SysPNode *plink);
void sys_pqueue_free_full(SysPQueue *queue, SysDestroyFunc free_func);

SYS_END_DECLS

#endif
