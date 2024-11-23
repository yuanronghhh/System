#ifndef __SYS_HSLIST_H__
#define __SYS_HSLIST_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define sys_hslist_foreach(list, node) \
    for(SysHsList *node = list;node;node = node->next)

#define SYS_HSLIST_CAST_TO(o, TypeName, member) SYS_HDATA_CAST_TO(o, TypeName, member)
#define SYS_HSLIST(o) SYS_HDATA(o)
#define SYS_HSLIST_CHECK(o) SYS_HDATA_CHECK(o)

typedef struct _SysHsList SysHsList;

struct _SysHsList {
  SysHData unowned;
  SysHsList *next;
};

/**
 * Singly linked lists
 */
SysHsList* sys_hslist_alloc(void);
void sys_hslist_init(SysHsList *list);
void sys_hslist_free(SysHsList *list);
void sys_hslist_free_1(SysHsList *list);
#define sys_hslist_free1 sys_hslist_free_1
void sys_hslist_free_full(SysHsList *list, SysDestroyFunc free_func);
SysHsList* sys_hslist_append (SysHsList   *list, SysHsList *new_list);
SysHsList* sys_hslist_prepend (SysHsList   *list, SysHsList  *new_list);
SysHsList* sys_hslist_insert (SysHsList   *list,
                SysHsList   *new_list,
                SysInt      position);
SysHsList* sys_hslist_insert_sorted(SysHsList *list, SysHsList *new_list, SysCompareFunc func);
SysHsList* sys_hslist_insert_sorted_with_data(SysHsList *list, SysHsList *new_list, SysCompareDataFunc func, SysPointer user_data);
SysHsList* sys_hslist_insert_before(SysHsList *hslist, SysHsList *sibling, SysHsList *new_list);
SysHsList* sys_hslist_concat(SysHsList *list1, SysHsList *list2);
SysHsList* sys_hslist_remove_link(SysHsList *list, SysHsList *link_);
SysHsList* sys_hslist_delete_link(SysHsList *list, SysHsList *link_);
SysHsList* sys_hslist_reverse(SysHsList *list);
SysHsList* sys_hslist_copy(SysHsList *list);

SysHsList* sys_hslist_copy_deep(SysHsList *list, SysCopyFunc func, SysPointer user_data);
SysHsList* sys_hslist_nth(SysHsList *list, SysUInt n);
SysHsList* sys_hslist_find_custom(SysHsList *list, const SysHsList *new_list, SysCompareFunc func);
SysInt sys_hslist_position(SysHsList *list, SysHsList *llink);
SysInt sys_hslist_index(SysHsList *list, const SysHsList *new_list);
SysHsList* sys_hslist_last(SysHsList *list);
SysUInt sys_hslist_length(SysHsList *list);
SysHsList* sys_hslist_sort(SysHsList *list, SysCompareFunc compare_func);
SysHsList* sys_hslist_sort_with_data(SysHsList *list, SysCompareDataFunc compare_func, SysPointer user_data);
SysHsList* sys_hslist_nth_data (SysHsList   *list,
                  SysUInt n);

void sys_clear_hslist(SysHsList **hslist_ptr, SysDestroyFunc destroy);

#define sys_clear_hslist(hslist_ptr, destroy) \
  do { \
    SysHsList *_hslist; \
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

#define sys_hslist_next(hslist) ((hslist) ? (((SysHsList *)(hslist))->next) : NULL)

SYS_END_DECLS

#endif
