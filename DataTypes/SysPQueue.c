#include <System/DataTypes/SysPQueue.h>

/* pqueue api */
static SysPNode *prio_p_node_new(SysInt prio, SysPointer data) {
  SysPNode *plink = sys_new0(SysPNode, 1);

  plink->parent.data = data;
  plink->prio = prio;

  return plink;
}

static void prio_list_free(SysPNode *plink) {
  sys_free(plink);
}

SysPQueue *sys_pqueue_new(void) {
  return sys_queue_new();
}

void sys_pqueue_destroy(SysPQueue *queue, SysDestroyFunc free_func) {
  sys_pqueue_foreach(queue, node) {
    free_func(node->data);
  }
  sys_pqueue_init(queue);
}

void sys_pqueue_init(SysPQueue *queue) {
  sys_queue_init(queue);
}

static SysBool _pqueue_push_tail(SysList *tail, SysPNode *plink) {
  SysList *nlist = SYS_LIST(plink);

  for (SysList *list = tail; list; list = list->prev) {
    SysPNode *plist = SYS_P_NODE(list);

    if (plink->prio > plist->prio) {
      if (list->next) {
        nlist->next = list->next;
        nlist->next->prev = nlist;
      }

      list->next = nlist;
      nlist->prev = list;

      return true;
    }
  }

  return false;
}

static SysBool _pqueue_push_head(SysList *head, SysPNode *plink) {
  SysList *nlist = SYS_LIST(plink);

  for (SysList *list = head; list; list = list->next) {
    SysPNode *plist = SYS_P_NODE(list);

    if (plink->prio <= plist->prio) {
      if (list->prev) {
        nlist->prev = list->prev;
        nlist->prev->next = nlist;
      }

      list->prev = nlist;
      nlist->next = list;

      return true;
    }
  }

  return false;
}

void sys_pqueue_push_head_link(SysPQueue *queue, SysPNode *plink) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(plink != NULL);
  sys_return_if_fail(plink->prio > 0);

  SysList *nlist = SYS_LIST(plink);

  sys_return_if_fail(nlist->prev == NULL);
  sys_return_if_fail(nlist->next == NULL);

  if (!_pqueue_push_head(queue->head, plink)) {
    if (queue->head) {
      nlist->next = queue->head;
      nlist->next->prev = nlist;
    }

    queue->head = nlist;
  }

  if (!queue->tail) {
    queue->tail = queue->head;
  }
}

void sys_pqueue_push_tail_link(SysPQueue *queue, SysPNode *plink) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(plink != NULL);
  sys_return_if_fail(plink->prio > 0);

  SysList *nlist = SYS_LIST(plink);

  sys_return_if_fail(nlist->prev == NULL);
  sys_return_if_fail(nlist->next == NULL);

  if (!_pqueue_push_tail(queue->tail, plink)) {
    if (queue->tail) {
      nlist->prev = queue->tail;
      nlist->prev->next = nlist;
    }

    queue->tail = nlist;
  }

  if (!queue->head) {
    queue->head = queue->tail;
  }
}

SysPNode *sys_pqueue_push_tail(SysPQueue *queue, SysInt prio, SysPointer data) {
  sys_return_val_if_fail(queue != NULL, NULL);
  sys_return_val_if_fail(prio > 0, NULL);

  SysPNode *plink = prio_p_node_new(prio, data);

  sys_pqueue_push_tail_link(queue, plink);

  if (queue->tail->next)
    queue->tail = queue->tail->next;
  else
    queue->head = queue->tail;
  queue->length++;

  return plink;
}

SysPNode *sys_pqueue_push_head(SysPQueue *queue, SysInt prio, SysPointer data) {
  sys_return_val_if_fail(queue != NULL, NULL);
  sys_return_val_if_fail(prio > 0, NULL);

  SysPNode *plink = prio_p_node_new(prio, data);

  sys_pqueue_push_head_link(queue, plink);

  if (!queue->tail)
    queue->tail = queue->head;
  queue->length++;

  return plink;
}

void sys_pqueue_unlink(SysPQueue *queue, SysPNode *plink) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(plink != NULL);
  sys_return_if_fail(plink->prio > 0);

  SysList *nlist = SYS_LIST(plink);

  if (nlist == queue->tail) {
    queue->tail = queue->tail->prev;
  }

  SysList *nlink = sys_list_remove_link(queue->head, nlist);

  queue->head = nlink;
  queue->length--;
}

void sys_pqueue_free(SysPQueue *queue) {
  sys_return_if_fail(queue != NULL);

  if (queue->head) {
    prio_list_free(SYS_P_NODE(queue->head));
    queue->head = NULL;
  }
  sys_slice_free(SysPQueue, queue);
}

void sys_pqueue_free_full(SysPQueue *queue, SysDestroyFunc free_func) {
  for (SysList *node = queue->head; node; node = node->next) {
    free_func(node->data);
  }

  sys_pqueue_free(queue);
}


