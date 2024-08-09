#ifndef __SYS_HSLIST_H__
#define __SYS_HSLIST_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define sys_hslist_foreach(list, node) \
    for(SysHSList *node = list;node;node = node->next)

typedef struct _SysHSList SysHSList;

struct _SysHSList {
 SysHSList *next;
};

/**
 * Singly linked lists
 */
SysHSList* sys_hslist_alloc(void);
void sys_hslist_free(SysHSList *list);
void sys_hslist_free_1(SysHSList *list);
#define sys_hslist_free1 sys_hslist_free_1
void sys_hslist_free_full(SysHSList *list, SysDestroyFunc free_func);
SysHSList* sys_hslist_append (SysHSList   *list, SysHSList *new_list);
SysHSList* sys_hslist_prepend (SysHSList   *list, SysHSList  *new_list);
SysHSList* sys_hslist_insert (SysHSList   *list,
                SysHSList   *new_list,
                SysInt      position);
SysHSList* sys_hslist_insert_sorted(SysHSList *list, SysHSList *new_list, SysCompareFunc func);
SysHSList* sys_hslist_insert_sorted_with_data(SysHSList *list, SysHSList *new_list, SysCompareDataFunc func, SysPointer user_data);
SysHSList* sys_hslist_insert_before(SysHSList *hslist, SysHSList *sibling, SysHSList *new_list);
SysHSList* sys_hslist_concat(SysHSList *list1, SysHSList *list2);
SysHSList* sys_hslist_remove_link(SysHSList *list, SysHSList *link_);
SysHSList* sys_hslist_delete_link(SysHSList *list, SysHSList *link_);
SysHSList* sys_hslist_reverse(SysHSList *list);
SysHSList* sys_hslist_copy(SysHSList *list);

SysHSList* sys_hslist_copy_deep(SysHSList *list, SysCopyFunc func, SysPointer user_data);
SysHSList* sys_hslist_nth(SysHSList *list, SysUInt n);
SysHSList* sys_hslist_find_custom(SysHSList *list, const SysHSList *new_list, SysCompareFunc func);
SysInt sys_hslist_position(SysHSList *list, SysHSList *llink);
SysInt sys_hslist_index(SysHSList *list, const SysHSList *new_list);
SysHSList* sys_hslist_last(SysHSList *list);
SysUInt sys_hslist_length(SysHSList *list);
SysHSList* sys_hslist_sort(SysHSList *list, SysCompareFunc compare_func);
SysHSList* sys_hslist_sort_with_data(SysHSList *list, SysCompareDataFunc compare_func, SysPointer user_data);
SysHSList* sys_hslist_nth_data (SysHSList   *list,
                  SysUInt n);

void sys_clear_hslist(SysHSList **hslist_ptr, SysDestroyFunc destroy);

#define sys_clear_hslist(hslist_ptr, destroy) \
  do { \
    SysHSList *_hslist; \
    \
    _hslist = *(hslist_ptr); \
    if (_hslist) \
    { \
      *hslist_ptr = NULL; \
      \
      if ((destroy) != NULL) \
      sys_hslist_free_full (_hslist, (destroy)); \
      else \
      sys_hslist_free (_hslist); \
    } \
  } while(0) \

#define sys_hslist_next(hslist) ((hslist) ? (((SysHSList *)(hslist))->next) : NULL)

SYS_END_DECLS

#endif
