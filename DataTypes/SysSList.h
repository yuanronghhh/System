#ifndef __SYS_SLIST_H__
#define __SYS_SLIST_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define sys_slist_foreach(list, node) \
    for(SysSList *node = list;node;node = node->next)

typedef struct _SysSList SysSList;

struct _SysSList {
 SysPointer data;
 SysSList *next;
};

/* Singly linked lists
*/
SysSList* sys_slist_alloc(void);
void sys_slist_free(SysSList *list);
void sys_slist_free_1(SysSList *list);
#define sys_slist_free1 sys_slist_free_1
void sys_slist_free_full(SysSList *list, SysDestroyFunc free_func);
SysSList* sys_slist_append(SysSList *list, SysPointer data);
SysSList* sys_slist_prepend(SysSList *list, SysPointer data);
SysSList* sys_slist_insert(SysSList *list, SysPointer data, SysInt position);
SysSList* sys_slist_insert_sorted(SysSList *list, SysPointer data, SysCompareFunc func);
SysSList* sys_slist_insert_sorted_with_data(SysSList *list, SysPointer data, SysCompareDataFunc func, SysPointer user_data);
SysSList* sys_slist_insert_before(SysSList *slist, SysSList *sibling, SysPointer data);
SysSList* sys_slist_concat(SysSList *list1, SysSList *list2);
SysSList* sys_slist_remove(SysSList *list, const SysPointer data);
SysSList* sys_slist_remove_all(SysSList *list, const SysPointer data);
SysSList* sys_slist_remove_link(SysSList *list, SysSList *link_);
SysSList* sys_slist_delete_link(SysSList *list, SysSList *link_);
SysSList* sys_slist_reverse(SysSList *list);
SysSList* sys_slist_copy(SysSList *list);

SysSList* sys_slist_copy_deep(SysSList *list, SysCopyFunc func, SysPointer user_data);
SysSList* sys_slist_nth(SysSList *list, SysUInt n);
SysSList* sys_slist_find(SysSList *list, const SysPointer data);
SysSList* sys_slist_find_custom(SysSList *list, const SysPointer data, SysCompareFunc func);
SysInt sys_slist_position(SysSList *list, SysSList *llink);
SysInt sys_slist_index(SysSList *list, const SysPointer data);
SysSList* sys_slist_last(SysSList *list);
SysUInt sys_slist_length(SysSList *list);
SysSList* sys_slist_sort(SysSList *list, SysCompareFunc compare_func);
SysSList* sys_slist_sort_with_data(SysSList *list, SysCompareDataFunc compare_func, SysPointer user_data);
SysPointer sys_slist_nth_data(SysSList *list, SysUInt n);

void sys_clear_slist(SysSList **slist_ptr, SysDestroyFunc destroy);

#define sys_clear_slist(slist_ptr, destroy) \
 do { \
 SysSList *_slist; \
 \
 _slist = *(slist_ptr); \
 if (_slist) \
 { \
 *slist_ptr = NULL; \
 \
 if ((destroy) != NULL) \
 sys_slist_free_full (_slist, (destroy)); \
 else \
 sys_slist_free (_slist); \
 } \
 } while(0) \

#define sys_slist_next(slist) ((slist) ? (((SysSList *)(slist))->next) : NULL)

SYS_END_DECLS

#endif
