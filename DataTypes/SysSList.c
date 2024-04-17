#include <System/DataTypes/SysSList.h>


/**
 * this code from glib slist
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

#define _sys_slist_alloc0()       sys_slice_new0 (SysSList)
#define _sys_slist_alloc()        sys_slice_new (SysSList)
#define _sys_slist_free1(slist)   sys_slice_free (SysSList, slist)

SysSList* sys_slist_alloc (void) {
  return _sys_slist_alloc0 ();
}

void sys_slist_free (SysSList *list) {
  sys_slice_free_chain (SysSList, list, next);
}

void sys_slist_free_1 (SysSList *list) {
  _sys_slist_free1 (list);
}

void sys_slist_free_full (SysSList         *list,
    SysDestroyFunc  free_func) {
  sys_slist_foreach (list, node) {
    free_func(node->data);
  }

  sys_slist_free (list);
}

SysSList* sys_slist_append (SysSList   *list,
                SysPointer  data) {
  SysSList *new_list;
  SysSList *last;

  new_list = _sys_slist_alloc ();
  new_list->data = data;
  new_list->next = NULL;

  if (list)
    {
      last = sys_slist_last (list);
      /* g_assert (last != NULL); */
      last->next = new_list;

      return list;
    }
  else
    return new_list;
}

SysSList* sys_slist_prepend (SysSList   *list,
                 SysPointer  data) {
  SysSList *new_list;

  new_list = _sys_slist_alloc ();
  new_list->data = data;
  new_list->next = list;

  return new_list;
}

SysSList* sys_slist_insert (SysSList   *list,
                SysPointer  data,
                SysInt      position) {
  SysSList *prev_list;
  SysSList *tmp_list;
  SysSList *new_list;

  if (position < 0)
    return sys_slist_append (list, data);
  else if (position == 0)
    return sys_slist_prepend (list, data);

  new_list = _sys_slist_alloc ();
  new_list->data = data;

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

SysSList* sys_slist_insert_before (SysSList  *slist,
                       SysSList  *sibling,
                       SysPointer data) {
  if (!slist)
    {
      slist = _sys_slist_alloc ();
      slist->data = data;
      slist->next = NULL;
      sys_return_val_if_fail (sibling == NULL, slist);
      return slist;
    }
  else
    {
      SysSList *node, *last = NULL;

      for (node = slist; node; last = node, node = last->next)
        if (node == sibling)
          break;
      if (!last)
        {
          node = _sys_slist_alloc ();
          node->data = data;
          node->next = slist;

          return node;
        }
      else
        {
          node = _sys_slist_alloc ();
          node->data = data;
          node->next = last->next;
          last->next = node;

          return slist;
        }
    }
}

SysSList * sys_slist_concat (SysSList *list1, SysSList *list2) {
  if (list2)
    {
      if (list1)
        sys_slist_last (list1)->next = list2;
      else
        list1 = list2;
    }

  return list1;
}

static SysSList* _sys_slist_remove_data (SysSList        *list,
                      const SysPointer  data,
                      SysBool       all) {
  SysSList *tmp = NULL;
  SysSList **previous_ptr = &list;

  while (*previous_ptr)
    {
      tmp = *previous_ptr;
      if (tmp->data == data)
        {
          *previous_ptr = tmp->next;
          sys_slist_free_1 (tmp);
          if (!all)
            break;
        }
      else
        {
          previous_ptr = &tmp->next;
        }
    }

  return list;
}
SysSList* sys_slist_remove (SysSList        *list,
                const SysPointer  data) {
  return _sys_slist_remove_data (list, data, false);
}

SysSList* sys_slist_remove_all (SysSList        *list,
                    const SysPointer  data) {
  return _sys_slist_remove_data (list, data, true);
}

static inline SysSList* _sys_slist_remove_link (SysSList *list,
                      SysSList *link) {
  SysSList *tmp = NULL;
  SysSList **previous_ptr = &list;

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

SysSList* sys_slist_remove_link (SysSList *list,
                     SysSList *link_) {
  return _sys_slist_remove_link (list, link_);
}

SysSList* sys_slist_delete_link (SysSList *list,
                     SysSList *link_) {
  list = _sys_slist_remove_link (list, link_);
  _sys_slist_free1 (link_);

  return list;
}

SysSList* sys_slist_copy (SysSList *list) {
  return sys_slist_copy_deep (list, NULL, NULL);
}

SysSList* sys_slist_copy_deep (SysSList *list,
    SysCopyFunc func, 
    SysPointer user_data) {
  SysSList *new_list = NULL;

  if (list)
    {
      SysSList *last;

      new_list = _sys_slist_alloc ();
      if (func)
        new_list->data = func (list->data, user_data);
      else
        new_list->data = list->data;
      last = new_list;
      list = list->next;
      while (list)
        {
          last->next = _sys_slist_alloc ();
          last = last->next;
          if (func)
            last->data = func (list->data, user_data);
          else
            last->data = list->data;
          list = list->next;
        }
      last->next = NULL;
    }

  return new_list;
}

SysSList* sys_slist_reverse (SysSList *list) {
  SysSList *prev = NULL;

  while (list)
    {
      SysSList *next = list->next;

      list->next = prev;

      prev = list;
      list = next;
    }

  return prev;
}

SysSList* sys_slist_nth (SysSList *list,
             SysUInt   n) {
  while (n-- > 0 && list)
    list = list->next;

  return list;
}

SysPointer sys_slist_nth_data (SysSList   *list,
                  SysUInt     n) {
  while (n-- > 0 && list)
    list = list->next;

  return list ? list->data : NULL;
}

SysSList* sys_slist_find (SysSList        *list,
              const SysPointer  data) {
  while (list)
    {
      if (list->data == data)
        break;
      list = list->next;
    }

  return list;
}


SysSList* sys_slist_find_custom (SysSList        *list,
                     const SysPointer  data,
                     SysCompareFunc   func) {
  sys_return_val_if_fail (func != NULL, list);

  while (list)
    {
      if (! func (list->data, data))
        return list;
      list = list->next;
    }

  return NULL;
}

SysInt sys_slist_position (SysSList *list,
                  SysSList *llink) {
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

SysInt sys_slist_index (SysSList        *list,
               const SysPointer  data) {
  SysInt i;

  i = 0;
  while (list)
    {
      if (list->data == data)
        return i;
      i++;
      list = list->next;
    }

  return -1;
}

SysSList* sys_slist_last (SysSList *list) {
  if (list)
    {
      while (list->next)
        list = list->next;
    }

  return list;
}

SysUInt sys_slist_length (SysSList *list) {
  SysUInt length;

  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }

  return length;
}

// void sys_slist_foreach (SysSList   *list,
//                  SysFunc     func,
//                  SysPointer  user_data) {
//   while (list)
//     {
//       SysSList *next = list->next;
//       (*func) (list->data, user_data);
//       list = next;
//     }
// }

static SysSList* sys_slist_insert_sorted_real (SysSList   *list,
                            SysPointer  data,
                            SysFunc     func,
                            SysPointer  user_data) {
  SysSList *tmp_list = list;
  SysSList *prev_list = NULL;
  SysSList *new_list;
  SysInt cmp;

  sys_return_val_if_fail (func != NULL, list);

  if (!list)
    {
      new_list = _sys_slist_alloc ();
      new_list->data = data;
      new_list->next = NULL;
      return new_list;
    }

  cmp = ((SysCompareDataFunc) func) (data, tmp_list->data, user_data);

  while ((tmp_list->next) && (cmp > 0))
    {
      prev_list = tmp_list;
      tmp_list = tmp_list->next;

      cmp = ((SysCompareDataFunc) func) (data, tmp_list->data, user_data);
    }

  new_list = _sys_slist_alloc ();
  new_list->data = data;

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

SysSList* sys_slist_insert_sorted (SysSList       *list,
                       SysPointer      data,
                       SysCompareFunc  func) {
  return sys_slist_insert_sorted_real (list, data, (SysFunc) func, NULL);
}

SysSList* sys_slist_insert_sorted_with_data (SysSList           *list,
                                 SysPointer          data,
                                 SysCompareDataFunc  func,
                                 SysPointer          user_data) {
  return sys_slist_insert_sorted_real (list, data, (SysFunc) func, user_data);
}

static SysSList * sys_slist_sort_merge (SysSList   *l1,
                    SysSList   *l2,
                    SysFunc     compare_func,
                    SysPointer  user_data) {
  SysSList list, *l;
  SysInt cmp;

  l=&list;

  while (l1 && l2)
    {
      cmp = ((SysCompareDataFunc) compare_func) (l1->data, l2->data, user_data);

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

static SysSList * sys_slist_sort_real (SysSList   *list,
                   SysFunc     compare_func,
                   SysPointer  user_data) {
  SysSList *l1, *l2;

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

  return sys_slist_sort_merge (
      sys_slist_sort_real (list, compare_func, user_data),
      sys_slist_sort_real (l2, compare_func, user_data),
      compare_func,
      user_data);
}

SysSList * sys_slist_sort (SysSList       *list,
              SysCompareFunc  compare_func) {
  return sys_slist_sort_real (list, (SysFunc) compare_func, NULL);
}

SysSList * sys_slist_sort_with_data (SysSList           *list,
                        SysCompareDataFunc  compare_func,
                        SysPointer          user_data) {
  return sys_slist_sort_real (list, (SysFunc) compare_func, user_data);
}

void (sys_clear_slist) (SysSList         **slist_ptr,
                 SysDestroyFunc   destroy) {
  SysSList *slist;

  slist = *slist_ptr;
  if (slist)
    {
      *slist_ptr = NULL;

      if (destroy)
        sys_slist_free_full (slist, destroy);
      else
        sys_slist_free (slist);
    }
}
