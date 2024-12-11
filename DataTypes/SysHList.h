#ifndef __SYS_HHLIST_H__
#define __SYS_HHLIST_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define SYS_HLIST(o) ((SysHList *)SYS_HDATA(o))

#define sys_hlist_foreach(list, node) for(SysHList *node = list;node;node = node->next)
#define sys_hlist_remove(list, new_list) sys_hlist_remove_link(list, new_list);

typedef struct _SysHList SysHList;

struct _SysHList {
  SysHData unowned;
  SysHList *next;
  SysHList *prev;
};

SYS_API void sys_hlist_init(SysHList *list);
SYS_API SysHList* sys_hlist_append(SysHList *list, SysHList *new_list);
SYS_API SysHList* sys_hlist_prepend(SysHList *list, SysHList *new_list);
SYS_API SysHList* sys_hlist_insert(SysHList *list, SysHList *new_list, SysInt      position);
SYS_API SysHList* sys_hlist_insert_before(SysHList *list, SysHList *sibling, SysHList *new_list);
SYS_API SysHList*   sys_hlist_concat(SysHList *list1, SysHList *list2);
SYS_API SysHList* sys_hlist_remove_all(SysHList *list, const SysHList* new_list);
SYS_API SysHList*   sys_hlist_remove_link(SysHList *list, SysHList *llink);
SYS_API SysHList*   sys_hlist_reverse(SysHList *list);
SYS_API SysHList*   sys_hlist_nth(SysHList *list, SysUInt  n);
SYS_API SysHList* sys_hlist_nth_prev(SysHList* list, SysUInt  n);
SYS_API SysHList* sys_hlist_nth_data(SysHList *list, SysUInt  n);
SYS_API SysHList* sys_hlist_find(SysHList *list, const SysHList* new_list);
SYS_API SysHList* sys_hlist_find_custom(SysHList *list, const SysHList *new_list, SysCompareFunc func);
SYS_API SysInt     sys_hlist_position(SysHList *list, SysHList *ulist);
SYS_API SysInt sys_hlist_index(SysHList *list, const SysHList* new_list);
SYS_API SysHList*   sys_hlist_last(SysHList *list);
SYS_API SysHList* sys_hlist_first(SysHList *list);
SYS_API SysUInt    sys_hlist_length(SysHList            *list);

SYS_END_DECLS

#endif
