#include <System/DataTypes/SysQueue.h>

/**
 * this code from glib GQueue
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

SysQueue* sys_queue_new(void) {
  return sys_slice_new0(SysQueue);
}

void sys_queue_free(SysQueue *queue) {
  sys_return_if_fail(queue != NULL);

  sys_list_free(queue->head);
  sys_slice_free(SysQueue, queue);
}

void sys_queue_free_full(SysQueue *queue, SysDestroyFunc free_func) {
  for (SysList *node = queue->head; node; node = node->next) {
    free_func(node->data);
  }

  sys_queue_free(queue);
}

void sys_queue_init(SysQueue *queue) {
  sys_return_if_fail(queue != NULL);

  queue->head = queue->tail = NULL;
  queue->length = 0;
}

void sys_queue_clear(SysQueue *queue) {
  sys_return_if_fail(queue != NULL);

  if (queue->head) {

    sys_list_free_1(queue->head);
  }
  sys_queue_init(queue);
}

SysBool sys_queue_is_empty(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, true);

  return queue->head == NULL;
}

SysUInt sys_queue_get_length(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, 0);

  return queue->length;
}

void sys_queue_reverse(SysQueue *queue) {
  sys_return_if_fail(queue != NULL);

  queue->tail = queue->head;
  queue->head = sys_list_reverse(queue->head);
}

SysQueue *sys_queue_copy(SysQueue *queue) {
  SysQueue *result;
  SysList *list;

  sys_return_val_if_fail(queue != NULL, NULL);

  result = sys_queue_new();

  for (list = queue->head; list != NULL; list = list->next)
    sys_queue_push_tail(result, list->data);

  return result;
}

SysList *sys_queue_find(SysQueue *queue, const SysPointer  data) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return sys_list_find(queue->head, data);
}

SysList *sys_queue_find_custom(SysQueue *queue,
    const SysPointer  data, 
    SysCompareFunc   func) {
  sys_return_val_if_fail(queue != NULL, NULL);
  sys_return_val_if_fail(func != NULL, NULL);

  return sys_list_find_custom(queue->head, data, func);
}

void sys_queue_push_head(SysQueue *queue, SysPointer  data) {
  sys_return_if_fail(queue != NULL);

  queue->head = sys_list_prepend(queue->head, data);
  if (!queue->tail)
    queue->tail = queue->head;
  queue->length++;
}

void sys_queue_push_nth(SysQueue *queue, SysPointer  data, SysUInt n) {
  sys_return_if_fail(queue != NULL);

  if (n < 0 || n >= queue->length) {
    sys_queue_push_tail(queue, data);
    return;
  }

  sys_queue_insert_before(queue, sys_queue_peek_nth_link(queue, n), data);
}

void sys_queue_push_head_link(SysQueue *queue, SysList *link) {
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

void sys_queue_push_tail(SysQueue *queue, SysPointer  data) {
  sys_return_if_fail(queue != NULL);

  queue->tail = sys_list_append(queue->tail, data);
  if (queue->tail->next)
    queue->tail = queue->tail->next;
  else
    queue->head = queue->tail;
  queue->length++;
}

void sys_queue_push_tail_link(SysQueue *queue, SysList *link) {
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

void sys_queue_push_nth_link(SysQueue *queue,
    SysUInt n,
    SysList *link_) {
  SysList *next;
  SysList *prev;

  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link_ != NULL);

  if (n < 0 || n >= queue->length) {
    sys_queue_push_tail_link(queue, link_);
    return;
  }

  sys_assert(queue->head);
  sys_assert(queue->tail);

  next = sys_queue_peek_nth_link(queue, n);
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

SysPointer sys_queue_pop_head(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  if (queue->head) {
    SysList *node = queue->head;
    SysPointer data = node->data;

    queue->head = node->next;
    if (queue->head)
      queue->head->prev = NULL;
    else
      queue->tail = NULL;
    sys_list_free_1(node);
    queue->length--;

    return data;
  }

  return NULL;
}

SysList *sys_queue_pop_head_link(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  if (queue->head) {
    SysList *node = queue->head;

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

SysList *sys_queue_peek_head_link(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return queue->head;
}

SysList *sys_queue_peek_tail_link(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return queue->tail;
}

SysPointer sys_queue_pop_tail(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  if (queue->tail) {
    SysList *node = queue->tail;
    SysPointer data = node->data;

    queue->tail = node->prev;
    if (queue->tail)
      queue->tail->next = NULL;
    else
      queue->head = NULL;
    queue->length--;
    sys_list_free_1(node);

    return data;
  }

  return NULL;
}

SysPointer sys_queue_pop_nth(SysQueue *queue, SysUInt   n) {
  SysList *nth_link;
  SysPointer result;

  sys_return_val_if_fail(queue != NULL, NULL);

  if (n >= queue->length)
    return NULL;

  nth_link = sys_queue_peek_nth_link(queue, n);
  result = nth_link->data;

  sys_queue_delete_link(queue, nth_link);

  return result;
}

SysList *sys_queue_pop_tail_link(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  if (queue->tail) {
    SysList *node = queue->tail;

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

SysList *sys_queue_pop_nth_link(SysQueue *queue, SysUInt   n) {
  SysList *link;

  sys_return_val_if_fail(queue != NULL, NULL);

  if (n >= queue->length)
    return NULL;

  link = sys_queue_peek_nth_link(queue, n);
  sys_queue_unlink(queue, link);

  return link;
}

SysList *sys_queue_peek_nth_link(SysQueue *queue, SysUInt   n) {
  SysList *link;
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

SysInt sys_queue_link_index(SysQueue *queue, SysList *link_) {
  sys_return_val_if_fail(queue != NULL, -1);

  return sys_list_position(queue->head, link_);
}

void sys_queue_unlink(SysQueue *queue, SysList *link_) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link_ != NULL);

  if (link_ == queue->tail)
    queue->tail = queue->tail->prev;

  queue->head = sys_list_remove_link(queue->head, link_);
  queue->length--;
}

void sys_queue_delete_link(SysQueue *queue, SysList *link_) {
  sys_return_if_fail(queue != NULL);
  sys_return_if_fail(link_ != NULL);

  sys_queue_unlink(queue, link_);
  sys_list_free_1(link_);
}

SysPointer sys_queue_peek_head(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return queue->head ? queue->head->data : NULL;
}

SysPointer sys_queue_peek_tail(SysQueue *queue) {
  sys_return_val_if_fail(queue != NULL, NULL);

  return queue->tail ? queue->tail->data : NULL;
}

SysPointer sys_queue_peek_nth(SysQueue *queue, SysUInt   n) {
  SysList *link;

  sys_return_val_if_fail(queue != NULL, NULL);

  link = sys_queue_peek_nth_link(queue, n);

  if (link)
    return link->data;

  return NULL;
}

SysInt sys_queue_index(SysQueue *queue, const SysPointer  data) {
  sys_return_val_if_fail(queue != NULL, -1);

  return sys_list_index(queue->head, data);
}

SysBool sys_queue_remove(SysQueue *queue, const SysPointer  data) {
  SysList *link;

  sys_return_val_if_fail(queue != NULL, false);

  link = sys_list_find(queue->head, data);

  if (link)
    sys_queue_delete_link(queue, link);

  return (link != NULL);
}

SysUInt sys_queue_remove_all(SysQueue *queue, const SysPointer  data) {
  SysList *list;
  SysUInt old_length;

  sys_return_val_if_fail(queue != NULL, 0);

  old_length = queue->length;

  list = queue->head;
  while (list) {
    SysList *next = list->next;

    if (list->data == data)
      sys_queue_delete_link(queue, list);

    list = next;
  }

  return (old_length - queue->length);
}

void sys_queue_insert_before(SysQueue *queue,
    SysList *sibling, 
    SysPointer  data) {
  sys_return_if_fail(queue != NULL);

  if (sibling == NULL) {
    sys_queue_push_tail(queue, data);
  } else {
    queue->head = sys_list_insert_before(queue->head, sibling, data);
    queue->length++;
  }
}

void sys_queue_insert_after(SysQueue *queue,
    SysList *sibling, 
    SysPointer  data) {
  sys_return_if_fail(queue != NULL);

  if (sibling == NULL)
    sys_queue_push_head(queue, data);
  else
    sys_queue_insert_before(queue, sibling->next, data);
}
