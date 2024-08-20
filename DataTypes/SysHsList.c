#include <System/DataTypes/SysHsList.h>

/**
 * license under GNU Lesser General Public
 */

#define _sys_hslist_alloc0()       sys_slice_new0 (SysHSList)
#define _sys_hslist_alloc()        sys_slice_new (SysHSList)
#define _sys_hslist_free1(hslist)   sys_slice_free (SysHSList, hslist)

SysHSList* sys_hslist_alloc (void) {
  return _sys_hslist_alloc0 ();
}

void sys_hslist_free (SysHSList *list) {
  sys_slice_free_chain (SysHSList, list, next);
}

void sys_hslist_free_1 (SysHSList *list) {
  _sys_hslist_free1 (list);
}

void sys_hslist_free_full (SysHSList         *list,
    SysDestroyFunc  free_func) {
  sys_hslist_foreach (list, node) {
    free_func(node);
  }
}

SysHSList* sys_hslist_append (SysHSList   *list,
                SysHSList *new_list) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  SysHSList *last;

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

SysHSList* sys_hslist_prepend (SysHSList   *list,
                 SysHSList  *new_list) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  new_list->next = list;

  return new_list;
}

SysHSList* sys_hslist_insert (SysHSList   *list,
                SysHSList   *new_list,
                SysInt      position) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  SysHSList *prev_list;
  SysHSList *tmp_list;

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

SysHSList* sys_hslist_insert_before (SysHSList  *hslist,
                       SysHSList  *sibling,
                       SysHSList *new_list) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  if (!hslist)
    {
      hslist->next = NULL;
      sys_return_val_if_fail (sibling == NULL, hslist);
      return hslist;
    }
  else
    {
      SysHSList *node, *last = NULL;

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

SysHSList * sys_hslist_concat (SysHSList *list1, SysHSList *list2) {
  if (list2)
    {
      if (list1)
        sys_hslist_last (list1)->next = list2;
      else
        list1 = list2;
    }

  return list1;
}

static inline SysHSList* _sys_hslist_remove_link (SysHSList *list,
                      SysHSList *link) {
  SysHSList *tmp = NULL;
  SysHSList **previous_ptr = &list;

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

SysHSList* sys_hslist_remove_link (SysHSList *list,
                     SysHSList *link_) {
  return _sys_hslist_remove_link (list, link_);
}

SysHSList* sys_hslist_delete_link (SysHSList *list,
                     SysHSList *link_) {
  list = _sys_hslist_remove_link (list, link_);
  _sys_hslist_free1 (link_);

  return list;
}

SysHSList* sys_hslist_copy (SysHSList *list) {
  return sys_hslist_copy_deep (list, NULL, NULL);
}

SysHSList* sys_hslist_copy_deep (SysHSList *list,
    SysCopyFunc func,
    SysPointer user_data) {
  SysHSList *new_list = NULL;
  SysHSList *last;

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

SysHSList* sys_hslist_reverse (SysHSList *list) {
  SysHSList *prev = NULL;

  while (list)
    {
      SysHSList *next = list->next;

      list->next = prev;

      prev = list;
      list = next;
    }

  return prev;
}

SysHSList* sys_hslist_nth (SysHSList *list,
             SysUInt   n) {
  while (n-- > 0 && list)
    list = list->next;

  return list;
}

SysHSList* sys_hslist_nth_data (SysHSList   *list,
                  SysUInt     n) {
  while (n-- > 0 && list)
    list = list->next;

  return list;
}

SysHSList* sys_hslist_find_custom (SysHSList        *list,
                     const SysHSList *new_list,
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

SysInt sys_hslist_position (SysHSList *list,
                  SysHSList *llink) {
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

SysInt sys_hslist_index (SysHSList        *list,
               const SysHSList *new_list) {
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

SysHSList* sys_hslist_last (SysHSList *list) {
  if (list)
    {
      while (list->next)
        list = list->next;
    }

  return list;
}

SysUInt sys_hslist_length (SysHSList *list) {
  SysUInt length;

  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }

  return length;
}

static SysHSList* sys_hslist_insert_sorted_real (SysHSList   *list,
                            SysHSList  *new_list,
                            SysFunc     func,
                            SysPointer  user_data) {
  sys_return_val_if_fail(new_list != NULL, NULL);

  SysHSList *tmp_list = list;
  SysHSList *prev_list = NULL;
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

SysHSList* sys_hslist_insert_sorted (SysHSList       *list,
                       SysHSList*     new_list,
                       SysCompareFunc  func) {
  return sys_hslist_insert_sorted_real (list, new_list, (SysFunc) func, NULL);
}

SysHSList* sys_hslist_insert_sorted_with_data (SysHSList           *list,
                                 SysHSList * new_list,
                                 SysCompareDataFunc  func,
                                 SysPointer          user_data) {
  return sys_hslist_insert_sorted_real (list, new_list, (SysFunc) func, user_data);
}

static SysHSList * sys_hslist_sort_merge (SysHSList   *l1,
                    SysHSList   *l2,
                    SysFunc     compare_func,
                    SysPointer  user_data) {
  SysHSList list, *l;
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

static SysHSList * sys_hslist_sort_real (SysHSList   *list,
                   SysFunc     compare_func,
                   SysPointer  user_data) {
  SysHSList *l1, *l2;

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

SysHSList * sys_hslist_sort (SysHSList       *list,
              SysCompareFunc  compare_func) {
  return sys_hslist_sort_real (list, (SysFunc) compare_func, NULL);
}

SysHSList * sys_hslist_sort_with_data (SysHSList           *list,
                        SysCompareDataFunc  compare_func,
                        SysPointer          user_data) {
  return sys_hslist_sort_real (list, (SysFunc) compare_func, user_data);
}

void (sys_clear_hslist) (SysHSList         **hslist_ptr,
                 SysDestroyFunc   destroy) {
  SysHSList *hslist;

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
