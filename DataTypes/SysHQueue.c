#include <System/DataTypes/SysHQueue.h>

/**
 * this code from glib GQueue
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

void sys_hqueue_init(SysHQueue *queue) {
  sys_return_if_fail(queue != NULL);

  queue->head = queue->tail = NULL;
  queue->length = 0;
}

void sys_hqueue_clear(SysHQueue *queue) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(sys_hqueue_is_empty(queue));

  sys_hqueue_init(queue);
}

SysBool sys_hqueue_is_empty(SysHQueue *queue) {
  sys_return_val_if_fail(queue != NULL, true);

  return queue->head == NULL;
}

SysUInt sys_hqueue_get_length(SysHQueue *queue) {
  sys_return_val_if_fail(queue != NULL, 0);

  return queue->length;
}

void sys_hqueue_reverse(SysHQueue *queue) {
  sys_return_if_fail(queue != NULL);

  queue->tail = queue->head;
  queue->head = sys_hlist_reverse(queue->head);
}

SysHList *sys_hqueue_find(SysHQueue *queue, const SysHList *new_list) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return sys_hlist_find(queue->head, new_list);
}

SysHList *sys_hqueue_find_custom(SysHQueue *queue,
    const SysHList *new_list, 
    SysCompareFunc   func) {
  sys_return_val_if_fail(queue != NULL, NULL);
  sys_return_val_if_fail(func != NULL, NULL);

  return sys_hlist_find_custom(queue->head, new_list, func);
}

void sys_hqueue_push_nth(SysHQueue *queue, SysHList *new_list, SysInt n) {
  sys_return_if_fail(queue != NULL);

  if (n < 0 || n >= (SysInt)queue->length) {
    sys_hqueue_push_tail(queue, new_list);
    return;
  }

  sys_hqueue_insert_before(queue, sys_hqueue_peek_nth_link(queue, n), new_list);
}

void sys_hqueue_push_head_link(SysHQueue *queue, SysHList *link) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link != NULL);
  sys_return_if_fail(link->prev == NULL);
  sys_return_if_fail(link->next == NULL);

  link->next = queue->head;
  if (queue->head)
    queue->head->prev = link;
  else
    queue->tail = link;
  queue->head = link;
  queue->length++;
}

void sys_hqueue_push_tail_link(SysHQueue *queue, SysHList *link) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link != NULL);
  sys_return_if_fail(link->prev == NULL);
  sys_return_if_fail(link->next == NULL);

  link->prev = queue->tail;
  if (queue->tail)
    queue->tail->next = link;
  else
    queue->head = link;
  queue->tail = link;
  queue->length++;
}

void sys_hqueue_push_nth_link(SysHQueue *queue,
    SysInt n,
    SysHList *link_) {
  SysHList *next;
  SysHList *prev;

  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link_ != NULL);

  if (n < 0 || n >= (SysInt)queue->length) {
    sys_hqueue_push_tail_link(queue, link_);
    return;
  }

  sys_assert(queue->head);
  sys_assert(queue->tail);

  next = sys_hqueue_peek_nth_link(queue, n);
  prev = next->prev;

  if (prev)
    prev->next = link_;
  next->prev = link_;

  link_->next = next;
  link_->prev = prev;

  if (queue->head->prev)
    queue->head = queue->head->prev;

  if (queue->tail->next)
    queue->tail = queue->tail->next;

  queue->length++;
}

SysHList *sys_hqueue_pop_head_link(SysHQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  if (queue->head) {
    SysHList *node = queue->head;

    queue->head = node->next;
    if (queue->head) {
      queue->head->prev = NULL;
      node->next = NULL;
    } else
      queue->tail = NULL;
    queue->length--;

    return node;
  }

  return NULL;
}

SysHList *sys_hqueue_peek_head_link(SysHQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return queue->head;
}

SysHList *sys_hqueue_peek_tail_link(SysHQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return queue->tail;
}

SysHList *sys_hqueue_pop_tail_link(SysHQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  if (queue->tail) {
    SysHList *node = queue->tail;

    queue->tail = node->prev;
    if (queue->tail) {
      queue->tail->next = NULL;
      node->prev = NULL;
    } else
      queue->head = NULL;
    queue->length--;

    return node;
  }

  return NULL;
}

SysHList *sys_hqueue_pop_nth_link(SysHQueue *queue, SysUInt   n) {
  SysHList *link;

  sys_return_val_if_fail(queue != NULL, NULL);

  if (n >= queue->length)
    return NULL;

  link = sys_hqueue_peek_nth_link(queue, n);
  sys_hqueue_unlink(queue, link);

  return link;
}

SysHList *sys_hqueue_peek_nth_link(SysHQueue *queue, SysUInt   n) {
  SysHList *link;
  SysUInt i;

  sys_return_val_if_fail(queue != NULL, NULL);

  if (n >= queue->length)
    return NULL;

  if (n > queue->length / 2) {
    n = queue->length - n - 1;

    link = queue->tail;
    for (i = 0; i < n; ++i)
      link = link->prev;
  } else {
    link = queue->head;
    for (i = 0; i < n; ++i)
      link = link->next;
  }

  return link;
}

SysInt sys_hqueue_link_index(SysHQueue *queue, SysHList *link_) {
  sys_return_val_if_fail(queue != NULL, -1);

  return sys_hlist_position(queue->head, link_);
}

void sys_hqueue_unlink(SysHQueue *queue, SysHList *link_) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link_ != NULL);

  if (link_ == queue->tail)
    queue->tail = queue->tail->prev;

  queue->head = sys_hlist_remove_link(queue->head, link_);
  queue->length--;
}

SysInt sys_hqueue_index(SysHQueue *queue, const SysHList *new_list) {
  sys_return_val_if_fail(queue != NULL, -1);

  return sys_hlist_index(queue->head, new_list);
}

void sys_hqueue_insert_before(SysHQueue *queue,
    SysHList *sibling, 
    SysHList *new_list) {
  sys_return_if_fail(queue != NULL);

  if (sibling == NULL) {
    sys_hqueue_push_tail(queue, new_list);
  } else {
    queue->head = sys_hlist_insert_before(queue->head, sibling, new_list);
    queue->length++;
  }
}

void sys_hqueue_insert_after(SysHQueue *queue,
    SysHList *sibling, 
    SysHList *new_list) {
  sys_return_if_fail(queue != NULL);

  if (sibling == NULL)
    sys_hqueue_push_head(queue, new_list);
  else
    sys_hqueue_insert_before(queue, sibling->next, new_list);
}
