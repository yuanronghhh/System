#ifndef __SYS_HNODE_H__
#define __SYS_HNODE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define  SYS_HNODE_IS_ROOT(hnode) (((SysHNode*) (hnode))->parent == NULL && \
     ((SysHNode*) (hnode))->prev == NULL && \
     ((SysHNode*) (hnode))->next == NULL)

#define SYS_HNODE_IS_LEAF(hnode) (((SysHNode*) (hnode))->children == NULL)

#define SYS_HNODE_CAST_TO(o, TypeName, member) SYS_HDATA_CAST_TO(o, TypeName, member)
#define SYS_HNODE(o) SYS_HDATA(o, SysHNode)
#define SYS_HNODE_CHECK(o) SYS_HDATA_CHECK(o)

typedef struct _SysHNode  SysHNode;

typedef SysBool (*SysHNodeTraverseFunc) (SysHNode        *hnode,
       SysPointer user_data);
typedef void  (*SysHNodeForeachFunc) (SysHNode        *hnode,
       SysPointer user_data);
typedef SysBool (*SysHNodeFunc) (SysHNode* node, SysPointer user_data);

struct _SysHNode
{
  SysHNode   *next;
  SysHNode   *prev;
  SysHNode   *parent;
  SysHNode   *children;

  /* <private> */
  SysHNode   *last_child;
  SysUInt check;
};

SYS_API SysBool sys_hnode_has_one_child(SysHNode *self);
SYS_API SysHNode*  sys_hnode_new  (void);
SYS_API void  sys_hnode_destroy  (SysHNode    *root);
SYS_API void  sys_hnode_unlink  (SysHNode    *hnode);
SYS_API SysHNode*   sys_hnode_copy_deep       (SysHNode            *hnode,
     SysCopyFunc         copy_func,
     SysPointer          user_data);
SYS_API SysHNode*   sys_hnode_copy            (SysHNode            *hnode);
SYS_API SysHNode*  sys_hnode_insert  (SysHNode    *parent,
     SysInt     position,
     SysHNode    *hnode);
SYS_API SysHNode*  sys_hnode_insert_before (SysHNode    *parent,
     SysHNode    *sibling,
     SysHNode    *hnode);
SYS_API SysHNode*   sys_hnode_insert_after    (SysHNode            *parent,
     SysHNode            *sibling,
     SysHNode            *hnode); 
SYS_API SysHNode*  sys_hnode_prepend  (SysHNode    *parent,
     SysHNode    *hnode);
SYS_API SysUInt  sys_hnode_n_hnodes  (SysHNode    *root,
     SysTraverseFlags    flags);
SYS_API SysHNode*  sys_hnode_get_root (SysHNode    *hnode);
SYS_API SysBool sys_hnode_is_ancestor (SysHNode    *hnode,
     SysHNode    *descendant);
SYS_API SysUInt  sys_hnode_depth  (SysHNode    *hnode);

SYS_API SysHNode* sys_hnode_append(SysHNode *parent, SysHNode *node);

SYS_API void  sys_hnode_traverse (SysHNode    *root,
     SysTraverseType    order,
     SysTraverseFlags    flags,
     SysInt     max_depth,
     SysHNodeTraverseFunc func,
     SysPointer    user_data);

SYS_API SysUInt  sys_hnode_max_height  (SysHNode *root);

SYS_API void  sys_hnode_children_foreach (SysHNode    *hnode,
      SysTraverseFlags   flags,
      SysHNodeForeachFunc func,
      SysPointer    user_data);
SYS_API void  sys_hnode_reverse_children (SysHNode    *hnode);
SYS_API SysUInt  sys_hnode_n_children  (SysHNode    *hnode);
SYS_API SysHNode*  sys_hnode_nth_child  (SysHNode    *hnode,
      SysUInt     n);
SYS_API SysHNode*  sys_hnode_last_child  (SysHNode    *hnode);
SYS_API SysInt  sys_hnode_child_position  (SysHNode    *hnode,
      SysHNode    *child);

SYS_API SysHNode*  sys_hnode_first_sibling  (SysHNode    *hnode);
SYS_API SysHNode*  sys_hnode_last_sibling  (SysHNode    *hnode);

#define  sys_hnode_prev_sibling(hnode) ((hnode) ? \
      ((SysHNode*) (hnode))->prev : NULL)

#define  sys_hnode_next_sibling(hnode) ((hnode) ? \
      ((SysHNode*) (hnode))->next : NULL)

#define  sys_hnode_first_child(hnode) ((hnode) ? \
      ((SysHNode*) (hnode))->children : NULL)

SYS_API void sys_hnode_set_last_child(SysHNode *self, SysHNode * last_child);
SYS_API SysHNode * sys_hnode_get_last_child(SysHNode *self);

SYS_API void sys_hnode_handle_bfs_r(SysHNode *self, SysHNodeFunc func, SysPointer user_data);
SYS_API void sys_hnode_handle_ff_r(SysHNode *self, SysHNodeFunc func, SysPointer user_data);
SYS_API void sys_hnode_handle_ft_r(SysHNode *self, SysHNodeFunc func, SysPointer user_data);
SYS_API void sys_hnode_set_parent(SysHNode *self, SysHNode *parent);
SYS_API SysHNode* sys_hnode_parent(SysHNode* self);
SYS_API SysHNode* sys_hnode_children(SysHNode* self);
SYS_API SysHNode* sys_hnode_next(SysHNode* self);
SYS_API SysHNode* sys_hnode_prev(SysHNode* self);
SYS_API void sys_hnode_init(SysHNode* node);

SYS_END_DECLS

#endif


