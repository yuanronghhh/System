#include <System/DataTypes/SysHList.h>

/**
 * this code from glib list
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

void sys_hlist_init(SysHList *list) {
  sys_hdata_init((SysHData *)list);
  list->next = NULL;
  list->prev = NULL;
}

void sys_hlist_free(SysHList *list) {
  sys_slice_free_chain(SysHList, list, next);
}

void sys_hlist_free_1(SysHList *list) {
  sys_slice_free(SysHList, list);
}

SysHList* sys_hlist_append(SysHList *list, SysHList *new_list) {
  SysHList *last;

  new_list->next = NULL;

  if (list) {
    last = sys_hlist_last(list);
    last->next = new_list;
    new_list->prev = last;

    return list;
  } else {
    new_list->prev = NULL;
    return new_list;
  }
}

SysHList* sys_hlist_prepend(SysHList *list, SysHList *new_list) {
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

SysHList* sys_hlist_insert(SysHList *list, SysHList *new_list, SysInt      position) {
  SysHList *tmp_list;

  if (position < 0)
    return sys_hlist_append(list, new_list);
  else if (position == 0)
    return sys_hlist_prepend(list, new_list);

  tmp_list = sys_hlist_nth(list, position);
  if (!tmp_list)
    return sys_hlist_append(list, new_list);

  new_list = sys_slice_new(SysHList);
  new_list->prev = tmp_list->prev;
  tmp_list->prev->next = new_list;
  new_list->next = tmp_list;
  tmp_list->prev = new_list;

  return list;
}

SysHList* sys_hlist_insert_before(SysHList *list, SysHList *sibling, SysHList *new_list) {
  if (!list) {
    sys_return_val_if_fail(sibling == NULL, list);

    return new_list;
  } else if (sibling) {
    SysHList *node;

    node = new_list;
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
    SysHList *last;

    last = list;
    while (last->next)
      last = last->next;

    last->next = new_list;
    last->next->prev = last;
    last->next->next = NULL;

    return list;
  }
}

SysHList* sys_hlist_concat(SysHList *list1, SysHList *list2) {
  SysHList *tmp_list;

  if (list2) {
    tmp_list = sys_hlist_last(list1);
    if (tmp_list)
      tmp_list->next = list2;
    else
      list1 = list2;
    list2->prev = tmp_list;
  }

  return list1;
}

static SYS_INLINE SysHList* _sys_hlist_remove_link(SysHList *list, SysHList *link) {
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

SysHList* sys_hlist_remove(SysHList *list, const SysHList* new_list) {
  SysHList *tmp;

  tmp = list;
  while (tmp) {
    if (tmp != new_list) {
      tmp = tmp->next;
    } else {
      list = _sys_hlist_remove_link(list, tmp);
      sys_slice_free(SysHList, tmp);

      break;
    }
  }
  return list;
}

SysHList* sys_hlist_remove_all(SysHList *list, const SysHList* new_list) {
  SysHList *tmp = list;

  while (tmp) {
    if (tmp != new_list)
      tmp = tmp->next;
    else {
      SysHList *next = tmp->next;

      if (tmp->prev)
        tmp->prev->next = next;
      else
        list = next;
      if (next)
        next->prev = tmp->prev;

      sys_slice_free(SysHList, tmp);
      tmp = next;
    }
  }
  return list;
}

SysHList* sys_hlist_remove_link(SysHList *list, SysHList *llink) {
  return _sys_hlist_remove_link(list, llink);
}

SysHList* sys_hlist_delete_link(SysHList *list, SysHList *link_) {
  list = _sys_hlist_remove_link(list, link_);
  sys_slice_free(SysHList, link_);

  return list;
}

SysHList* sys_hlist_reverse(SysHList *list) {
  SysHList *last;

  last = NULL;
  while (list) {
    last = list;
    list = last->next;
    last->next = last->prev;
    last->prev = list;
  }

  return last;
}

SysHList* sys_hlist_nth(SysHList *list, SysUInt  n) {
  while ((n-- > 0) && list)
    list = list->next;

  return list;
}

SysHList* sys_hlist_nth_prev(SysHList *list, SysUInt  n) {
  while ((n-- > 0) && list)
    list = list->prev;

  return list;
}

SysHList* sys_hlist_nth_data(SysHList *list, SysUInt  n) {
  while ((n-- > 0) && list) {
    list = list->next;
  }

  return list ? list : NULL;
}

SysHList* sys_hlist_find(SysHList *list, const SysHList* new_list) {
  while (list) {
    if (list == new_list)
      break;
    list = list->next;
  }

  return list;
}

SysHList* sys_hlist_find_custom(SysHList *list, const SysHList *new_list, SysCompareFunc func) {
  sys_return_val_if_fail(func != NULL, list);

  while (list) {
    if (!func(list, new_list)) {
      return list;
    }
    list = list->next;
  }

  return NULL;
}

SysInt sys_hlist_position(SysHList *list, SysHList *llink) {
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

SysInt sys_hlist_index(SysHList *list, const SysHList* new_list) {
  SysInt i;

  i = 0;
  while (list) {
    if (list == new_list)
      return i;
    i++;
    list = list->next;
  }

  return -1;
}

SysHList* sys_hlist_last(SysHList *list) {
  if (list) {
    while (list->next)
      list = list->next;
  }

  return list;
}

SysHList* sys_hlist_first(SysHList *list) {
  if (list) {
    while (list->prev)
      list = list->prev;
  }

  return list;
}

SysUInt sys_hlist_length(SysHList *list) {
  SysUInt length;

  length = 0;
  while (list) {
    length++;
    list = list->next;
  }

  return length;
}
