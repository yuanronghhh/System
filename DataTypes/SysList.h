#ifndef __SYS_LIST_H__
#define __SYS_LIST_H__

#include <Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define sys_list_foreach(list, node)        \
    for(SysList *node = list;node;node = node->next) {          \

#define sys_list_foreach_end }

typedef struct _SysList SysList;

struct _SysList {
  SysPointer data;
  SysList *next;
  SysList *prev;
};

SYS_API SysList*   sys_list_new(void);
SYS_API void       sys_list_free_1(SysList *list);
SYS_API void       sys_list_free(SysList *list);
SYS_API void       sys_list_free_full(SysList *list, SysDestroyFunc func);
SYS_API SysList*   sys_list_append(SysList *list, SysPointer data);
SYS_API SysList*   sys_list_prepend(SysList *list, SysPointer data);
SYS_API SysList*   sys_list_insert(SysList *list, SysPointer data,SysInt position);
SYS_API SysList*   sys_list_insert_before(SysList *list, SysList *sibling, SysPointer  data);
SYS_API SysList*   sys_list_concat(SysList *list1, SysList *list2);
SYS_API SysList*   sys_list_remove(SysList *list, SysPointer data);
SYS_API SysList*   sys_list_remove_all(SysList *list, const SysPointer data);
SYS_API SysList*   sys_list_remove_link(SysList *list, SysList *llink);
SYS_API SysList*   sys_list_delete_link(SysList *list,SysList *link_);
SYS_API SysList*   sys_list_reverse(SysList *list);
SYS_API SysList*   sys_list_nth(SysList *list, SysUInt  n);
SYS_API SysList*   sys_list_find(SysList *list, const SysPointer data);
SYS_API SysList*   sys_list_find_custom(SysList *list, const SysPointer data,SysCompareFunc func);
SYS_API SysInt     sys_list_position(SysList *list, SysList *ulist);
SYS_API SysInt     sys_list_index(SysList *list, const SysPointer  data);
SYS_API SysList*   sys_list_last(SysList *list);
SYS_API SysUInt    sys_list_length(SysList            *list);

SYS_END_DECLS

#endif
