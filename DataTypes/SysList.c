#include <System/DataTypes/SysList.h>

/**
 * this code from glib list
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

SysList* sys_list_new(void) {
  SysList *list = sys_new0_N(SysList, 1);
  return list;
}

void sys_list_free_full(SysList  *list, SysDestroyFunc func) {
  sys_list_foreach(list, node) {
    func(node->data);
  };

  sys_list_free(list);
}

void sys_list_free(SysList *list) {
  sys_slice_free_chain(SysList, list, next);
}

void sys_list_free_1(SysList *list) {
  sys_slice_free(SysList, list);
}

SysList* sys_list_append(SysList *list, SysPointer  data) {
  SysList *new_list;
  SysList *last;

  new_list = sys_slice_new(SysList);
  new_list->data = data;
  new_list->next = NULL;

  if (list) {
    last = sys_list_last(list);
    last->next = new_list;
    new_list->prev = last;

    return list;
  } else {
    new_list->prev = NULL;
    return new_list;
  }
}

SysList* sys_list_prepend(SysList *list, SysPointer  data) {
  SysList *new_list;

  new_list = sys_slice_new(SysList);
  new_list->data = data;
  new_list->next = list;

  if (list) {
    new_list->prev = list->prev;
    if (list->prev)
      list->prev->next = new_list;
    list->prev = new_list;
  } else
    new_list->prev = NULL;

  return new_list;
}

SysList* sys_list_insert(SysList *list, SysPointer  data, SysInt      position) {
  SysList *new_list;
  SysList *tmp_list;

  if (position < 0)
    return sys_list_append(list, data);
  else if (position == 0)
    return sys_list_prepend(list, data);

  tmp_list = sys_list_nth(list, position);
  if (!tmp_list)
    return sys_list_append(list, data);

  new_list = sys_slice_new(SysList);
  new_list->data = data;
  new_list->prev = tmp_list->prev;
  tmp_list->prev->next = new_list;
  new_list->next = tmp_list;
  tmp_list->prev = new_list;

  return list;
}

SysList* sys_list_insert_before(SysList *list, SysList *sibling, SysPointer  data) {
  if (!list) {
    list = sys_slice_new(SysList);
    list->data = data;
    sys_return_val_if_fail(sibling == NULL, list);
    return list;
  } else if (sibling) {
    SysList *node;

    node = sys_slice_new(SysList);
    node->data = data;
    node->prev = sibling->prev;
    node->next = sibling;
    sibling->prev = node;
    if (node->prev) {
      node->prev->next = node;
      return list;
    } else {
      sys_return_val_if_fail(sibling == list, node);
      return node;
    }
  } else {
    SysList *last;

    last = list;
    while (last->next)
      last = last->next;

    last->next = sys_slice_new(SysList);
    last->next->data = data;
    last->next->prev = last;
    last->next->next = NULL;

    return list;
  }
}

SysList* sys_list_concat(SysList *list1, SysList *list2) {
  SysList *tmp_list;

  if (list2) {
    tmp_list = sys_list_last(list1);
    if (tmp_list)
      tmp_list->next = list2;
    else
      list1 = list2;
    list2->prev = tmp_list;
  }

  return list1;
}

static SYS_INLINE SysList* _sys_list_remove_link(SysList *list, SysList *link) {
  if (link == NULL)
    return list;

  if (link->prev) {
    if (link->prev->next == link) {
      link->prev->next = link->next;
    } else {
      sys_warning_N("%s", "corrupted double-linked list detected");
    }
  }
  if (link->next) {
    if (link->next->prev == link) {
      link->next->prev = link->prev;
    } else {
      sys_warning_N("%s", "corrupted double-linked list detected");
    }
  }

  if (link == list) {
    list = list->next;
  }

  link->next = NULL;
  link->prev = NULL;

  return list;
}

SysList* sys_list_remove(SysList *list, const SysPointer  data) {
  SysList *tmp;

  tmp = list;
  while (tmp) {
    if (tmp->data != data) {
      tmp = tmp->next;
    } else {
      list = _sys_list_remove_link(list, tmp);
      sys_slice_free(SysList, tmp);

      break;
    }
  }
  return list;
}

SysList* sys_list_remove_all(SysList *list, const SysPointer  data) {
  SysList *tmp = list;

  while (tmp) {
    if (tmp->data != data)
      tmp = tmp->next;
    else {
      SysList *next = tmp->next;

      if (tmp->prev)
        tmp->prev->next = next;
      else
        list = next;
      if (next)
        next->prev = tmp->prev;

      sys_slice_free(SysList, tmp);
      tmp = next;
    }
  }
  return list;
}

SysList* sys_list_remove_link(SysList *list, SysList *llink) {
  return _sys_list_remove_link(list, llink);
}

SysList* sys_list_delete_link(SysList *list, SysList *link_) {
  list = _sys_list_remove_link(list, link_);
  sys_slice_free(SysList, link_);

  return list;
}

SysList* sys_list_reverse(SysList *list) {
  SysList *last;

  last = NULL;
  while (list) {
    last = list;
    list = last->next;
    last->next = last->prev;
    last->prev = list;
  }

  return last;
}

SysList* sys_list_nth(SysList *list, SysUInt  n) {
  while ((n-- > 0) && list)
    list = list->next;

  return list;
}

SysList* sys_list_nth_prev(SysList *list, SysUInt  n) {
  while ((n-- > 0) && list)
    list = list->prev;

  return list;
}

SysPointer sys_list_nth_data(SysList *list, SysUInt  n) {
  while ((n-- > 0) && list) {
    list = list->next;
  }

  return list ? list->data : NULL;
}

SysList* sys_list_find(SysList *list, const SysPointer  data) {
  while (list) {
    if (list->data == data)
      break;
    list = list->next;
  }

  return list;
}

SysList* sys_list_find_custom(SysList *list, const SysPointer  data, SysCompareFunc func) {
  sys_return_val_if_fail(func != NULL, list);

  while (list) {
    if (!func(list->data, data)) {
      return list;
    }
    list = list->next;
  }

  return NULL;
}

SysInt sys_list_position(SysList *list, SysList *llink) {
  SysInt i;

  i = 0;
  while (list) {
    if (list == llink)
      return i;
    i++;
    list = list->next;
  }

  return -1;
}

SysInt sys_list_index(SysList *list, const SysPointer  data) {
  SysInt i;

  i = 0;
  while (list) {
    if (list->data == data)
      return i;
    i++;
    list = list->next;
  }

  return -1;
}

SysList* sys_list_last(SysList *list) {
  if (list) {
    while (list->next)
      list = list->next;
  }

  return list;
}

SysList* sys_list_first(SysList *list) {
  if (list) {
    while (list->prev)
      list = list->prev;
  }

  return list;
}

SysUInt sys_list_length(SysList *list) {
  SysUInt length;

  length = 0;
  while (list) {
    length++;
    list = list->next;
  }

  return length;
}
