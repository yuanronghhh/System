#ifndef __SYS_TREE_H__
#define __SYS_TREE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysTree  SysTree;
typedef struct _SysTreeNode SysTreeNode;

typedef SysBool (*SysTraverseFunc) (SysPointer  key,
                                   SysPointer  value,
                                   SysPointer  data);


typedef SysBool (*SysTraverseNodeFunc) (SysTreeNode *node,
                                       SysPointer   data);

SYS_API SysTree*   sys_tree_new             (SysCompareFunc      key_compare_func);
SYS_API SysTree*   sys_tree_new_with_data   (SysCompareDataFunc  key_compare_func,
                                 SysPointer          key_compare_data);
SYS_API SysTree*   sys_tree_new_full        (SysCompareDataFunc  key_compare_func,
                                 SysPointer          key_compare_data,
                                 SysDestroyFunc    key_destroy_func,
                                 SysDestroyFunc    value_destroy_func);
SYS_API SysTreeNode *sys_tree_node_first (SysTree *tree);
SYS_API SysTreeNode *sys_tree_node_last (SysTree *tree);
SYS_API SysTreeNode *sys_tree_node_previous (SysTreeNode *node);
SYS_API SysTreeNode *sys_tree_node_next (SysTreeNode *node);
SYS_API SysTree*   sys_tree_ref             (SysTree            *tree);
SYS_API void     sys_tree_unref           (SysTree            *tree);
SYS_API void     sys_tree_destroy         (SysTree            *tree);
SYS_API SysTreeNode *sys_tree_insert_node (SysTree *tree,
                               SysPointer key,
                               SysPointer value);
SYS_API void     sys_tree_insert          (SysTree            *tree,
                                 SysPointer          key,
                                 SysPointer          value);
SYS_API SysTreeNode *sys_tree_replace_node (SysTree *tree,
                                SysPointer key,
                                SysPointer value);
SYS_API void     sys_tree_replace         (SysTree            *tree,
                                 SysPointer          key,
                                 SysPointer          value);
SYS_API SysBool sys_tree_remove          (SysTree            *tree,
                                 const SysPointer     key);

SYS_API void     sys_tree_remove_all      (SysTree            *tree);

SYS_API SysBool sys_tree_steal           (SysTree            *tree,
                                 const SysPointer     key);
SYS_API SysPointer sys_tree_node_key (SysTreeNode *node);
SYS_API SysPointer sys_tree_node_value (SysTreeNode *node);
SYS_API SysTreeNode *sys_tree_lookup_node (SysTree *tree,
                               const SysPointer key);
SYS_API SysPointer sys_tree_lookup          (SysTree            *tree,
                                 const SysPointer     key);
SYS_API SysBool sys_tree_lookup_extended (SysTree            *tree,
                                 const SysPointer     lookup_key,
                                 SysPointer         *orisys_key,
                                 SysPointer         *value);
SYS_API void     sys_tree_foreach         (SysTree            *tree,
                                 SysTraverseFunc	   func,
                                 SysPointer	   user_data);
SYS_API void sys_tree_foreach_node (SysTree *tree,
                          SysTraverseNodeFunc func,
                          SysPointer user_data);

SYS_API SysTreeNode *sys_tree_search_node (SysTree *tree,
                               SysCompareFunc search_func,
                               const SysPointer user_data);
SYS_API SysPointer sys_tree_search          (SysTree            *tree,
                                 SysCompareFunc      search_func,
                                 const SysPointer     user_data);
SYS_API SysTreeNode *sys_tree_lower_bound (SysTree *tree,
                               const SysPointer key);
SYS_API SysTreeNode *sys_tree_upper_bound (SysTree *tree,
                               const SysPointer key);
SYS_API SysInt     sys_tree_height          (SysTree            *tree);
SYS_API SysInt     sys_tree_nnodes          (SysTree            *tree);

SYS_END_DECLS

#endif
