#include <System/DataTypes/SysHsList.h>

/**
 * license under GNU Lesser General Public
 */

#define _sys_hslist_alloc0()       sys_slice_new0 (SysHsList)
#define _sys_hslist_alloc()        sys_slice_new (SysHsList)
#define _sys_hslist_free1(hslist)   sys_slice_free (SysHsList, hslist)

SysHsList* sys_hslist_alloc (void) {
  return _sys_hslist_alloc0 ();
}

void sys_hslist_init(SysHsList *list) {
  sys_hdata_init((SysHData *)list);
  list->next = NULL;
}

void sys_hslist_free (SysHsList *list) {
  sys_slice_free_chain (SysHsList, list, next);
}

void sys_hslist_free_1 (SysHsList *list) {
  _sys_hslist_free1 (list);
}

void sys_hslist_free_full (SysHsList         *list,
    SysDestroyFunc  free_func) {
  sys_hslist_foreach (list, node) {
    free_func(node);
  }
}

SysHsList* sys_hslist_append (SysHsList   *list,
                SysHsList *new_list) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  SysHsList *last;

  new_list->next = NULL;

  if (list)
    {
      last = sys_hslist_last (list);
      last->next = new_list;

      return list;
    }
  else
    return new_list;
}

SysHsList* sys_hslist_prepend (SysHsList   *list,
                 SysHsList  *new_list) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  new_list->next = list;

  return new_list;
}

SysHsList* sys_hslist_insert (SysHsList   *list,
                SysHsList   *new_list,
                SysInt      position) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  SysHsList *prev_list;
  SysHsList *tmp_list;

  if (position < 0)
    return sys_hslist_append (list, new_list);
  else if (position == 0)
    return sys_hslist_prepend (list, new_list);

  if (!list)
    {
      new_list->next = NULL;
      return new_list;
    }

  prev_list = NULL;
  tmp_list = list;

  while ((position-- > 0) && tmp_list)
  {
    prev_list = tmp_list;
    tmp_list = tmp_list->next;
  }

  new_list->next = prev_list->next;
  prev_list->next = new_list;

  return list;
}

SysHsList* sys_hslist_insert_before (SysHsList  *hslist,
                       SysHsList  *sibling,
                       SysHsList *new_list) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  if (!hslist)
    {
      hslist->next = NULL;
      sys_return_val_if_fail (sibling == NULL, hslist);
      return hslist;
    }
  else
    {
      SysHsList *node, *last = NULL;

      for (node = hslist; node; last = node, node = last->next)
        if (node == sibling)
          break;
      if (!last)
        {
          node = new_list;
          node->next = hslist;

          return node;
        }
      else
        {
          node = new_list;
          node->next = last->next;
          last->next = node;

          return hslist;
        }
    }
}

SysHsList * sys_hslist_concat (SysHsList *list1, SysHsList *list2) {
  if (list2)
    {
      if (list1)
        sys_hslist_last (list1)->next = list2;
      else
        list1 = list2;
    }

  return list1;
}

static inline SysHsList* _sys_hslist_remove_link (SysHsList *list,
                      SysHsList *link) {
  SysHsList *tmp = NULL;
  SysHsList **previous_ptr = &list;

  while (*previous_ptr)
    {
      tmp = *previous_ptr;
      if (tmp == link)
        {
          *previous_ptr = tmp->next;
          tmp->next = NULL;
          break;
        }

      previous_ptr = &tmp->next;
    }

  return list;
}

SysHsList* sys_hslist_remove_link (SysHsList *list,
                     SysHsList *link_) {
  return _sys_hslist_remove_link (list, link_);
}

SysHsList* sys_hslist_delete_link (SysHsList *list,
                     SysHsList *link_) {
  list = _sys_hslist_remove_link (list, link_);
  _sys_hslist_free1 (link_);

  return list;
}

SysHsList* sys_hslist_copy (SysHsList *list) {
  return sys_hslist_copy_deep (list, NULL, NULL);
}

SysHsList* sys_hslist_copy_deep (SysHsList *list,
    SysCopyFunc func,
    SysPointer user_data) {
  SysHsList *new_list = NULL;
  SysHsList *last;

  sys_return_val_if_fail(list != NULL, NULL);
  sys_return_val_if_fail(func != NULL, NULL);

  new_list = func(list, user_data);
  sys_return_val_if_fail(new_list != NULL, NULL);

  last = new_list;
  list = list->next;

  while (list) {
    last->next = list;
    last = last->next;
    last = func (list, user_data);

    list = list->next;
  }
  last->next = NULL;

  return new_list;
}

SysHsList* sys_hslist_reverse (SysHsList *list) {
  SysHsList *prev = NULL;

  while (list)
    {
      SysHsList *next = list->next;

      list->next = prev;

      prev = list;
      list = next;
    }

  return prev;
}

SysHsList* sys_hslist_nth (SysHsList *list,
             SysUInt   n) {
  while (n-- > 0 && list)
    list = list->next;

  return list;
}

SysHsList* sys_hslist_nth_data (SysHsList   *list,
                  SysUInt     n) {
  while (n-- > 0 && list)
    list = list->next;

  return list;
}

SysHsList* sys_hslist_find_custom (SysHsList        *list,
                     const SysHsList *new_list,
                     SysCompareFunc   func) {
  sys_return_val_if_fail (func != NULL, list);

  while (list)
    {
      if (! func (list, new_list))
        return list;
      list = list->next;
    }

  return NULL;
}

SysInt sys_hslist_position (SysHsList *list,
                  SysHsList *llink) {
  SysInt i;

  i = 0;
  while (list)
    {
      if (list == llink)
        return i;
      i++;
      list = list->next;
    }

  return -1;
}

SysInt sys_hslist_index (SysHsList        *list,
               const SysHsList *new_list) {
  SysInt i;

  i = 0;
  while (list)
    {
      if (list == new_list)
        return i;
      i++;
      list = list->next;
    }

  return -1;
}

SysHsList* sys_hslist_last (SysHsList *list) {
  if (list)
    {
      while (list->next)
        list = list->next;
    }

  return list;
}

SysUInt sys_hslist_length (SysHsList *list) {
  SysUInt length;

  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }

  return length;
}

static SysHsList* sys_hslist_insert_sorted_real (SysHsList   *list,
                            SysHsList  *new_list,
                            SysFunc     func,
                            SysPointer  user_data) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  SysHsList *tmp_list = list;
  SysHsList *prev_list = NULL;
  SysInt cmp;

  sys_return_val_if_fail (func != NULL, list);

  if (!list)
    {
      new_list->next = NULL;
      return new_list;
    }

  cmp = ((SysCompareDataFunc) func) (new_list, tmp_list, user_data);

  while ((tmp_list->next) && (cmp > 0))
    {
      prev_list = tmp_list;
      tmp_list = tmp_list->next;

      cmp = ((SysCompareDataFunc) func) (new_list, tmp_list, user_data);
    }

  if ((!tmp_list->next) && (cmp > 0))
    {
      tmp_list->next = new_list;
      new_list->next = NULL;
      return list;
    }

  if (prev_list)
    {
      prev_list->next = new_list;
      new_list->next = tmp_list;
      return list;
    }
  else
    {
      new_list->next = list;
      return new_list;
    }
}

SysHsList* sys_hslist_insert_sorted (SysHsList       *list,
                       SysHsList*     new_list,
                       SysCompareFunc  func) {
  return sys_hslist_insert_sorted_real (list, new_list, (SysFunc) func, NULL);
}

SysHsList* sys_hslist_insert_sorted_with_data (SysHsList           *list,
                                 SysHsList * new_list,
                                 SysCompareDataFunc  func,
                                 SysPointer          user_data) {
  return sys_hslist_insert_sorted_real (list, new_list, (SysFunc) func, user_data);
}

static SysHsList * sys_hslist_sort_merge (SysHsList   *l1,
                    SysHsList   *l2,
                    SysFunc     compare_func,
                    SysPointer  user_data) {
  SysHsList list, *l;
  SysInt cmp;

  l=&list;

  while (l1 && l2)
    {
      cmp = ((SysCompareDataFunc) compare_func) (l1, l2, user_data);

      if (cmp <= 0)
        {
          l=l->next=l1;
          l1=l1->next;
        }
      else
        {
          l=l->next=l2;
          l2=l2->next;
        }
    }
  l->next= l1 ? l1 : l2;

  return list.next;
}

static SysHsList * sys_hslist_sort_real (SysHsList   *list,
                   SysFunc     compare_func,
                   SysPointer  user_data) {
  SysHsList *l1, *l2;

  if (!list)
    return NULL;
  if (!list->next)
    return list;

  l1 = list;
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL)
        break;
      l1=l1->next;
    }
  l2 = l1->next;
  l1->next = NULL;

  return sys_hslist_sort_merge (
      sys_hslist_sort_real (list, compare_func, user_data),
      sys_hslist_sort_real (l2, compare_func, user_data),
      compare_func,
      user_data);
}

SysHsList * sys_hslist_sort (SysHsList       *list,
              SysCompareFunc  compare_func) {
  return sys_hslist_sort_real (list, (SysFunc) compare_func, NULL);
}

SysHsList * sys_hslist_sort_with_data (SysHsList           *list,
                        SysCompareDataFunc  compare_func,
                        SysPointer          user_data) {
  return sys_hslist_sort_real (list, (SysFunc) compare_func, user_data);
}

void (sys_clear_hslist) (SysHsList         **hslist_ptr,
                 SysDestroyFunc   destroy) {
  SysHsList *hslist;

  hslist = *hslist_ptr;
  if (hslist)
    {
      *hslist_ptr = NULL;

      if (destroy)
        sys_hslist_free_full (hslist, destroy);
      else
        sys_hslist_free (hslist);
    }
}
