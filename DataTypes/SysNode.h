#ifndef __SYS_NODE_H__
#define __SYS_NODE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysNode  SysNode;

typedef enum
{
  SYS_TRAVERSE_LEAVES     = 1 << 0,
  SYS_TRAVERSE_NON_LEAVES = 1 << 1,
  SYS_TRAVERSE_ALL        = SYS_TRAVERSE_LEAVES | SYS_TRAVERSE_NON_LEAVES,
  SYS_TRAVERSE_MASK       = 0x03,
  SYS_TRAVERSE_LEAFS      = SYS_TRAVERSE_LEAVES,
  SYS_TRAVERSE_NON_LEAFS  = SYS_TRAVERSE_NON_LEAVES
} SysTraverseFlags;

typedef enum
{
  SYS_IN_ORDER,
  SYS_PRE_ORDER,
  SYS_POST_ORDER,
  SYS_LEVEL_ORDER
} SysTraverseType;

typedef SysBool (*SysNodeTraverseFunc) (SysNode        *node,
       SysPointer data);
typedef void  (*SysNodeForeachFunc) (SysNode        *node,
       SysPointer data);

struct _SysNode
{
  SysPointer data;
  SysNode   *next;
  SysNode   *prev;
  SysNode   *parent;
  SysNode   *children;
};

#define  SYS_NODE_IS_ROOT(node) (((SysNode*) (node))->parent == NULL && \
     ((SysNode*) (node))->prev == NULL && \
     ((SysNode*) (node))->next == NULL)

#define  SYS_NODE_IS_LEAF(node) (((SysNode*) (node))->children == NULL)

SYS_API SysNode*  sys_node_new  (SysPointer    data);
SYS_API void  sys_node_destroy  (SysNode    *root);
SYS_API void  sys_node_unlink  (SysNode    *node);
SYS_API SysNode*   sys_node_copy_deep       (SysNode            *node,
     SysCopyFunc         copy_func,
     SysPointer          data);
SYS_API SysNode*   sys_node_copy            (SysNode            *node);
SYS_API SysNode*  sys_node_insert  (SysNode    *parent,
     SysInt     position,
     SysNode    *node);
SYS_API SysNode*  sys_node_insert_before (SysNode    *parent,
     SysNode    *sibling,
     SysNode    *node);
SYS_API SysNode*   sys_node_insert_after    (SysNode            *parent,
     SysNode            *sibling,
     SysNode            *node); 
SYS_API SysNode*  sys_node_prepend  (SysNode    *parent,
     SysNode    *node);
SYS_API SysUInt  sys_node_n_nodes  (SysNode    *root,
     SysTraverseFlags    flags);
SYS_API SysNode*  sys_node_get_root (SysNode    *node);
SYS_API SysBool sys_node_is_ancestor (SysNode    *node,
     SysNode    *descendant);
SYS_API SysUInt  sys_node_depth  (SysNode    *node);
SYS_API SysNode*  sys_node_find  (SysNode    *root,
     SysTraverseType    order,
     SysTraverseFlags    flags,
     SysPointer    data);

#define sys_node_append(parent, node)    \
     sys_node_insert_before ((parent), NULL, (node))

#define sys_node_insert_data(parent, position, data)  \
     sys_node_insert ((parent), (position), sys_node_new (data))

#define sys_node_insert_data_after(parent, sibling, data) \
     sys_node_insert_after ((parent), (sibling), sys_node_new (data))

#define sys_node_insert_data_before(parent, sibling, data) \
     sys_node_insert_before ((parent), (sibling), sys_node_new (data))

#define sys_node_prepend_data(parent, data)   \
     sys_node_prepend ((parent), sys_node_new (data))

#define sys_node_append_data(parent, data)   \
     sys_node_insert_before ((parent), NULL, sys_node_new (data))

SYS_API void  sys_node_traverse (SysNode    *root,
     SysTraverseType    order,
     SysTraverseFlags    flags,
     SysInt     max_depth,
     SysNodeTraverseFunc func,
     SysPointer    data);

SYS_API SysUInt  sys_node_max_height  (SysNode *root);

SYS_API void  sys_node_children_foreach (SysNode    *node,
      SysTraverseFlags   flags,
      SysNodeForeachFunc func,
      SysPointer    data);
SYS_API void  sys_node_reverse_children (SysNode    *node);
SYS_API SysUInt  sys_node_n_children  (SysNode    *node);
SYS_API SysNode*  sys_node_nth_child  (SysNode    *node,
      SysUInt     n);
SYS_API SysNode*  sys_node_last_child  (SysNode    *node);
SYS_API SysNode*  sys_node_find_child  (SysNode    *node,
      SysTraverseFlags   flags,
      SysPointer    data);
SYS_API SysInt  sys_node_child_position  (SysNode    *node,
      SysNode    *child);
SYS_API SysInt  sys_node_child_index  (SysNode    *node,
      SysPointer    data);

SYS_API SysNode*  sys_node_first_sibling  (SysNode    *node);
SYS_API SysNode*  sys_node_last_sibling  (SysNode    *node);

#define  sys_node_prev_sibling(node) ((node) ? \
      ((SysNode*) (node))->prev : NULL)

#define  sys_node_next_sibling(node) ((node) ? \
      ((SysNode*) (node))->next : NULL)

#define  sys_node_first_child(node) ((node) ? \
      ((SysNode*) (node))->children : NULL)

SYS_END_DECLS

#endif
