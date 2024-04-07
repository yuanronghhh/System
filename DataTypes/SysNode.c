/**
 * this code from glib GNode
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

#include <System/DataTypes/SysNode.h>

#define sys_node_alloc0()         sys_slice_new0 (SysNode)
#define sys_node_free(node)       sys_slice_free (SysNode, node)

SysNode* sys_node_new (SysPointer data) {
  SysNode *node = sys_node_alloc0 ();
  node->data = data;
  return node;
}

static void sys_nodes_free (SysNode *node) {
  while (node) {
    SysNode *next = node->next;
    if (node->children)
      sys_nodes_free (node->children);
    sys_node_free (node);
    node = next;
  }
}

void sys_node_destroy (SysNode *root) {
  sys_return_if_fail (root != NULL);

  if (!SYS_NODE_IS_ROOT (root))
    sys_node_unlink (root);

  sys_nodes_free (root);
}

void sys_node_unlink (SysNode *node) {
  sys_return_if_fail (node != NULL);

  if (node->prev)
    node->prev->next = node->next;
  else if (node->parent)
    node->parent->children = node->next;
  node->parent = NULL;
  if (node->next) {
    node->next->prev = node->prev;
    node->next = NULL;
  }
  node->prev = NULL;
}

SysNode* sys_node_copy_deep (SysNode     *node, 
    SysCopyFunc  copy_func,
    SysPointer   data) {
  SysNode *new_node = NULL;

  if (copy_func == NULL)
    return sys_node_copy (node);

  if (node) {
    SysNode *child, *new_child;

    new_node = sys_node_new (copy_func (node->data, data));

    for (child = sys_node_last_child (node); child; child = child->prev)  {
      new_child = sys_node_copy_deep (child, copy_func, data);
      sys_node_prepend (new_node, new_child);
    }
  }

  return new_node;
}

SysNode* sys_node_copy (SysNode *node) {
  SysNode *new_node = NULL;

  if (node) {
    SysNode *child;

    new_node = sys_node_new (node->data);

    for (child = sys_node_last_child (node); child; child = child->prev)
      sys_node_prepend (new_node, sys_node_copy (child));
  }

  return new_node;
}

SysNode* sys_node_insert (SysNode *parent,
    SysInt   position,
    SysNode *node) {
  sys_return_val_if_fail (parent != NULL, node);
  sys_return_val_if_fail (node != NULL, node);
  sys_return_val_if_fail (SYS_NODE_IS_ROOT (node), node);

  if (position > 0)
    return sys_node_insert_before (parent,
        sys_node_nth_child (parent, position),
        node);
  else if (position == 0)
    return sys_node_prepend (parent, node);
  else /* if (position < 0) */
    return sys_node_append (parent, node);
}

SysNode* sys_node_insert_before (SysNode *parent,
    SysNode *sibling,
    SysNode *node) {
  sys_return_val_if_fail (parent != NULL, node);
  sys_return_val_if_fail (node != NULL, node);
  sys_return_val_if_fail (SYS_NODE_IS_ROOT (node), node);
  if (sibling)
    sys_return_val_if_fail (sibling->parent == parent, node);

  node->parent = parent;

  if (sibling) {
    if (sibling->prev) {
      node->prev = sibling->prev;
      node->prev->next = node;
      node->next = sibling;
      sibling->prev = node;
    } else {
      node->parent->children = node;
      node->next = sibling;
      sibling->prev = node;
    }
  } else {
    if (parent->children) {
      sibling = parent->children;
      while (sibling->next)
        sibling = sibling->next;
      node->prev = sibling;
      sibling->next = node;
    } else
      node->parent->children = node;
  }

  return node;
}

SysNode* sys_node_insert_after (SysNode *parent,
    SysNode *sibling,
    SysNode *node) {
  sys_return_val_if_fail (parent != NULL, node);
  sys_return_val_if_fail (node != NULL, node);
  sys_return_val_if_fail (SYS_NODE_IS_ROOT (node), node);
  if (sibling)
    sys_return_val_if_fail (sibling->parent == parent, node);

  node->parent = parent;

  if (sibling) {
    if (sibling->next) {
      sibling->next->prev = node;
    }
    node->next = sibling->next;
    node->prev = sibling;
    sibling->next = node;
  } else {
    if (parent->children) {
      node->next = parent->children;
      parent->children->prev = node;
    }
    parent->children = node;
  }

  return node;
}

SysNode* sys_node_prepend (SysNode *parent,
    SysNode *node) {
  sys_return_val_if_fail (parent != NULL, node);

  return sys_node_insert_before (parent, parent->children, node);
}

SysNode* sys_node_get_root (SysNode *node) {
  sys_return_val_if_fail (node != NULL, NULL);

  while (node->parent)
    node = node->parent;

  return node;
}

SysBool sys_node_is_ancestor (SysNode *node,
    SysNode *descendant) {
  sys_return_val_if_fail (node != NULL, false);
  sys_return_val_if_fail (descendant != NULL, false);

  while (descendant) {
    if (descendant->parent == node)
      return true;

    descendant = descendant->parent;
  }

  return false;
}

SysUInt sys_node_depth (SysNode *node) {
  SysUInt depth = 0;

  while (node) {
    depth++;
    node = node->parent;
  }

  return depth;
}

void sys_node_reverse_children (SysNode *node) {
  SysNode *child;
  SysNode *last;

  sys_return_if_fail (node != NULL);

  child = node->children;
  last = NULL;
  while (child) {
    last = child;
    child = last->next;
    last->next = last->prev;
    last->prev = child;
  }
  node->children = last;
}

SysUInt sys_node_max_height (SysNode *root) {
  SysNode *child;
  SysUInt max_height = 0;

  if (!root)
    return 0;

  child = root->children;
  while (child) {
    SysUInt tmp_height;

    tmp_height = sys_node_max_height (child);
    if (tmp_height > max_height)
      max_height = tmp_height;
    child = child->next;
  }

  return max_height + 1;
}

static SysBool sys_node_traverse_pre_order (SysNode     *node,
    SysTraverseFlags    flags,
    SysNodeTraverseFunc func,
    SysPointer      data) {
  if (node->children) {
    SysNode *child;

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (node, data))
      return true;

    child = node->children;
    while (child) {
      SysNode *current;

      current = child;
      child = current->next;
      if (sys_node_traverse_pre_order (current, flags, func, data))
        return true;
    }
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (node, data))
    return true;

  return false;
}

static SysBool sys_node_depth_traverse_pre_order (SysNode    *node,
    SysTraverseFlags    flags,
    SysUInt     depth,
    SysNodeTraverseFunc func,
    SysPointer    data) {
  if (node->children) {
    SysNode *child;

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (node, data))
      return true;

    depth--;
    if (!depth)
      return false;

    child = node->children;
    while (child) {
      SysNode *current;

      current = child;
      child = current->next;
      if (sys_node_depth_traverse_pre_order (current, flags, depth, func, data))
        return true;
    }
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (node, data))
    return true;

  return false;
}

static SysBool sys_node_traverse_post_order (SysNode      *node,
    SysTraverseFlags    flags,
    SysNodeTraverseFunc func,
    SysPointer       data) {
  if (node->children) {
    SysNode *child;

    child = node->children;
    while (child) {
      SysNode *current;

      current = child;
      child = current->next;
      if (sys_node_traverse_post_order (current, flags, func, data))
        return true;
    }

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (node, data))
      return true;

  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (node, data))
    return true;

  return false;
}

static SysBool sys_node_depth_traverse_post_order (SysNode     *node,
    SysTraverseFlags    flags,
    SysUInt      depth,
    SysNodeTraverseFunc func,
    SysPointer     data) {
  if (node->children) {
    depth--;
    if (depth) {
      SysNode *child;

      child = node->children;
      while (child) {
        SysNode *current;

        current = child;
        child = current->next;
        if (sys_node_depth_traverse_post_order (current, flags, depth, func, data))
          return true;
      }
    }

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (node, data))
      return true;

  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (node, data))
    return true;

  return false;
}

static SysBool sys_node_traverse_in_order (SysNode     *node,
    SysTraverseFlags    flags,
    SysNodeTraverseFunc func,
    SysPointer     data) {
  if (node->children) {
    SysNode *child;
    SysNode *current;

    child = node->children;
    current = child;
    child = current->next;

    if (sys_node_traverse_in_order (current, flags, func, data))
      return true;

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (node, data))
      return true;

    while (child) {
      current = child;
      child = current->next;
      if (sys_node_traverse_in_order (current, flags, func, data))
        return true;
    }
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (node, data))
    return true;

  return false;
}

static SysBool sys_node_depth_traverse_in_order (SysNode   *node,
    SysTraverseFlags   flags,
    SysUInt    depth,
    SysNodeTraverseFunc func,
    SysPointer   data) {
  if (node->children) {
    depth--;
    if (depth) {
      SysNode *child;
      SysNode *current;

      child = node->children;
      current = child;
      child = current->next;

      if (sys_node_depth_traverse_in_order (current, flags, depth, func, data))
        return true;

      if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
          func (node, data))
        return true;

      while (child) {
        current = child;
        child = current->next;
        if (sys_node_depth_traverse_in_order (current, flags, depth, func, data))
          return true;
      }
    } else if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (node, data))
      return true;
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (node, data))
    return true;

  return false;
}

static SysBool sys_node_traverse_level (SysNode   *node,
    SysTraverseFlags   flags,
    SysUInt    level,
    SysNodeTraverseFunc  func,
    SysPointer           data,
    SysBool          *more_levels) {
  if (level == 0)  {
    if (node->children) {
      *more_levels = true;
      return (flags & SYS_TRAVERSE_NON_LEAFS) && func (node, data);
    } else {
      return (flags & SYS_TRAVERSE_LEAFS) && func (node, data);
    }
  } else  {
    node = node->children;

    while (node) {
      if (sys_node_traverse_level (node, flags, level - 1, func, data, more_levels))
        return true;

      node = node->next;
    }
  }

  return false;
}

static SysBool sys_node_depth_traverse_level (SysNode             *node,
    SysTraverseFlags flags,
    SysInt  depth,
    SysNodeTraverseFunc  func,
    SysPointer         data) {
  SysUInt level;
  SysBool more_levels;

  level = 0;  
  while (depth < 0 || level != (SysUInt) depth) {
    more_levels = false;
    if (sys_node_traverse_level (node, flags, level, func, data, &more_levels))
      return true;
    if (!more_levels)
      break;
    level++;
  }
  return false;
}

void sys_node_traverse (SysNode    *root,
    SysTraverseType    order,
    SysTraverseFlags    flags,
    SysInt     depth,
    SysNodeTraverseFunc func,
    SysPointer    data) {
  sys_return_if_fail (root != NULL);
  sys_return_if_fail (func != NULL);
  sys_return_if_fail (order <= SYS_LEVEL_ORDER);
  sys_return_if_fail (flags <= SYS_TRAVERSE_MASK);
  sys_return_if_fail (depth == -1 || depth > 0);

  switch (order) {
    case SYS_PRE_ORDER:
      if (depth < 0)
        sys_node_traverse_pre_order (root, flags, func, data);
      else
        sys_node_depth_traverse_pre_order (root, flags, depth, func, data);
      break;
    case SYS_POST_ORDER:
      if (depth < 0)
        sys_node_traverse_post_order (root, flags, func, data);
      else
        sys_node_depth_traverse_post_order (root, flags, depth, func, data);
      break;
    case SYS_IN_ORDER:
      if (depth < 0)
        sys_node_traverse_in_order (root, flags, func, data);
      else
        sys_node_depth_traverse_in_order (root, flags, depth, func, data);
      break;
    case SYS_LEVEL_ORDER:
      sys_node_depth_traverse_level (root, flags, depth, func, data);
      break;
  }
}

static SysBool sys_node_find_func (SysNode    *node,
    SysPointer  data) {
  SysPointer *d = data;

  if (*d != node->data)
    return false;

  *(++d) = node;

  return true;
}

SysNode* sys_node_find (SysNode     *root,
    SysTraverseType   order,
    SysTraverseFlags  flags,
    SysPointer        data) {
  SysPointer d[2];

  sys_return_val_if_fail (root != NULL, NULL);
  sys_return_val_if_fail (order <= SYS_LEVEL_ORDER, NULL);
  sys_return_val_if_fail (flags <= SYS_TRAVERSE_MASK, NULL);

  d[0] = data;
  d[1] = NULL;

  sys_node_traverse (root, order, flags, -1, sys_node_find_func, d);

  return d[1];
}

static void sys_node_count_func (SysNode  *node,
    SysTraverseFlags flags,
    SysUInt  *n) {
  if (node->children) {
    SysNode *child;

    if (flags & SYS_TRAVERSE_NON_LEAFS)
      (*n)++;

    child = node->children;
    while (child) {
      sys_node_count_func (child, flags, n);
      child = child->next;
    }
  } else if (flags & SYS_TRAVERSE_LEAFS)
    (*n)++;
}

SysUInt sys_node_n_nodes (SysNode        *root,
    SysTraverseFlags  flags) {
  SysUInt n = 0;

  sys_return_val_if_fail (root != NULL, 0);
  sys_return_val_if_fail (flags <= SYS_TRAVERSE_MASK, 0);

  sys_node_count_func (root, flags, &n);

  return n;
}

SysNode* sys_node_last_child (SysNode *node) {
  sys_return_val_if_fail (node != NULL, NULL);

  node = node->children;
  if (node)
    while (node->next)
      node = node->next;

  return node;
}

SysNode* sys_node_nth_child (SysNode *node,
    SysUInt  n) {
  sys_return_val_if_fail (node != NULL, NULL);

  node = node->children;
  if (node)
    while ((n-- > 0) && node)
      node = node->next;

  return node;
}

SysUInt sys_node_n_children (SysNode *node) {
  SysUInt n = 0;

  sys_return_val_if_fail (node != NULL, 0);

  node = node->children;
  while (node) {
    n++;
    node = node->next;
  }

  return n;
}

SysNode* sys_node_find_child (SysNode   *node,
    SysTraverseFlags  flags,
    SysPointer    data) {
  sys_return_val_if_fail (node != NULL, NULL);
  sys_return_val_if_fail (flags <= SYS_TRAVERSE_MASK, NULL);

  node = node->children;
  while (node) {
    if (node->data == data) {
      if (SYS_NODE_IS_LEAF (node)) {
        if (flags & SYS_TRAVERSE_LEAFS)
          return node;
      } else {
        if (flags & SYS_TRAVERSE_NON_LEAFS)
          return node;
      }
    }
    node = node->next;
  }

  return NULL;
}

SysInt sys_node_child_position (SysNode *node,
    SysNode *child) {
  SysUInt n = 0;

  sys_return_val_if_fail (node != NULL, -1);
  sys_return_val_if_fail (child != NULL, -1);
  sys_return_val_if_fail (child->parent == node, -1);

  node = node->children;
  while (node) {
    if (node == child)
      return n;
    n++;
    node = node->next;
  }

  return -1;
}

SysInt sys_node_child_index (SysNode    *node,
    SysPointer  data) {
  SysUInt n = 0;

  sys_return_val_if_fail (node != NULL, -1);

  node = node->children;
  while (node) {
    if (node->data == data)
      return n;
    n++;
    node = node->next;
  }

  return -1;
}

SysNode* sys_node_first_sibling (SysNode *node) {
  sys_return_val_if_fail (node != NULL, NULL);

  if (node->parent)
    return node->parent->children;

  while (node->prev)
    node = node->prev;

  return node;
}

SysNode* sys_node_last_sibling (SysNode *node) {
  sys_return_val_if_fail (node != NULL, NULL);

  while (node->next)
    node = node->next;

  return node;
}

void sys_node_children_foreach (SysNode    *node,
    SysTraverseFlags    flags,
    SysNodeForeachFunc  func,
    SysPointer    data) {
  sys_return_if_fail (node != NULL);
  sys_return_if_fail (flags <= SYS_TRAVERSE_MASK);
  sys_return_if_fail (func != NULL);

  node = node->children;
  while (node) {
    SysNode *current;

    current = node;
    node = current->next;
    if (SYS_NODE_IS_LEAF (current)) {
      if (flags & SYS_TRAVERSE_LEAFS)
        func (current, data);
    } else {
      if (flags & SYS_TRAVERSE_NON_LEAFS)
        func (current, data);
    }
  }
}
