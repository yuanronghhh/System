#include <System/DataTypes/SysHNode.h>
#include <System/DataTypes/SysQueue.h>


#define CHECK_VALUE 0xdeadbeef
#define sys_hnode_alloc0()         sys_slice_new0 (SysHNode)
#define sys_hnode_free(hnode)       sys_slice_free (SysHNode, hnode)
#define HNODE_CHECK(hnode) (hnode != NULL && (hnode)->check == CHECK_VALUE)

SysHNode* sys_hnode_new (void) {
  SysHNode *hnode = sys_hnode_alloc0 ();
  return hnode;
}

void sys_hnode_init(SysHNode* node) {
  node->check = CHECK_VALUE;
  node->children = NULL;
  node->last_child = NULL;
  node->parent = NULL;
  node->prev = NULL;
}

SysHNode *_sys_hnode_cast_check(SysHNode* self) {
  if (self == NULL) { return NULL; }
  sys_return_val_if_fail(HNODE_CHECK(self), NULL);

  return self;
}

SysPointer _sys_hnode_cast_to(SysHNode *o, SysSize offsize) {
  if(o == NULL) { return NULL; }
  sys_return_val_if_fail(HNODE_CHECK(o), NULL);

  return ((char*)o - offsize);
}

static void sys_hnodes_free (SysHNode *hnode) {
  while (hnode) {
    SysHNode *next = hnode->next;
    if (hnode->children)
      sys_hnodes_free (hnode->children);
    sys_hnode_free (hnode);
    hnode = next;
  }
}

void sys_hnode_destroy (SysHNode *root) {
  sys_return_if_fail (root != NULL);

  if (!SYS_HNODE_IS_ROOT (root))
    sys_hnode_unlink (root);

  sys_hnodes_free (root);
}

void sys_hnode_unlink (SysHNode *hnode) {
  sys_return_if_fail (hnode != NULL);

  if (hnode->prev)
    hnode->prev->next = hnode->next;
  else if (hnode->parent)
    hnode->parent->children = hnode->next;
  hnode->parent = NULL;
  if (hnode->next) {
    hnode->next->prev = hnode->prev;
    hnode->next = NULL;
  }
  hnode->prev = NULL;
}

SysHNode* sys_hnode_copy_deep (SysHNode     *hnode,
    SysCopyFunc  copy_func,
    SysPointer   user_data) {
  SysHNode *new_hnode = NULL;

  if (copy_func == NULL)
    return sys_hnode_copy (hnode);

  if (hnode) {
    SysHNode *child, *new_child;

    new_hnode = copy_func (hnode, user_data);

    for (child = sys_hnode_last_child (hnode); child; child = child->prev)  {
      new_child = sys_hnode_copy_deep (child, copy_func, user_data);
      sys_hnode_prepend (new_hnode, new_child);
    }
  }

  return new_hnode;
}

SysHNode* sys_hnode_copy (SysHNode *hnode) {
  SysHNode *new_hnode = NULL;

  if (hnode) {
    SysHNode *child;

    new_hnode = sys_hnode_new ();

    for (child = sys_hnode_last_child (hnode); child; child = child->prev)
      sys_hnode_prepend (new_hnode, sys_hnode_copy (child));
  }

  return new_hnode;
}

SysHNode* sys_hnode_insert (SysHNode *parent,
    SysInt   position,
    SysHNode *hnode) {
  sys_return_val_if_fail (parent != NULL, hnode);
  sys_return_val_if_fail (hnode != NULL, hnode);
  sys_return_val_if_fail (SYS_HNODE_IS_ROOT (hnode), hnode);

  if (position > 0)
    return sys_hnode_insert_before (parent,
        sys_hnode_nth_child (parent, position),
        hnode);
  else if (position == 0)
    return sys_hnode_prepend (parent, hnode);
  else /* if (position < 0) */
    return sys_hnode_append (parent, hnode);
}

SysHNode* sys_hnode_insert_before (SysHNode *parent,
    SysHNode *sibling,
    SysHNode *hnode) {
  sys_return_val_if_fail (parent != NULL, hnode);
  sys_return_val_if_fail (hnode != NULL, hnode);
  sys_return_val_if_fail (SYS_HNODE_IS_ROOT (hnode), hnode);
  if (sibling)
    sys_return_val_if_fail (sibling->parent == parent, hnode);

  hnode->parent = parent;

  if (sibling) {
    if (sibling->prev) {
      hnode->prev = sibling->prev;
      hnode->prev->next = hnode;
      hnode->next = sibling;
      sibling->prev = hnode;
    } else {
      hnode->parent->children = hnode;
      hnode->next = sibling;
      sibling->prev = hnode;
    }
  } else {
    if (parent->children) {
      sibling = parent->children;
      while (sibling->next)
        sibling = sibling->next;
      hnode->prev = sibling;
      sibling->next = hnode;
    } else
      hnode->parent->children = hnode;
  }

  return hnode;
}

SysHNode* sys_hnode_insert_after (SysHNode *parent,
    SysHNode *sibling,
    SysHNode *hnode) {
  sys_return_val_if_fail (HNODE_CHECK(parent), hnode);
  sys_return_val_if_fail (HNODE_CHECK(hnode), hnode);
  sys_return_val_if_fail (SYS_HNODE_IS_ROOT (hnode), hnode);
  if (sibling)
    sys_return_val_if_fail (sibling->parent == parent, hnode);

  hnode->parent = parent;

  if (sibling) {
    if (sibling->next) {
      sibling->next->prev = hnode;
    }
    hnode->next = sibling->next;
    hnode->prev = sibling;
    sibling->next = hnode;
  } else {
    if (parent->children) {
      hnode->next = parent->children;
      parent->children->prev = hnode;
    }
    parent->children = hnode;
  }

  return hnode;
}

SysHNode* sys_hnode_prepend (SysHNode *parent,
    SysHNode *hnode) {
  sys_return_val_if_fail (HNODE_CHECK(parent), hnode);

  return sys_hnode_insert_before (parent, parent->children, hnode);
}

SysHNode* sys_hnode_get_root (SysHNode *hnode) {
  sys_return_val_if_fail (hnode != NULL, NULL);

  while (hnode->parent)
    hnode = hnode->parent;

  return hnode;
}

SysBool sys_hnode_is_ancestor (SysHNode *hnode,
    SysHNode *descendant) {
  sys_return_val_if_fail (HNODE_CHECK(hnode), false);
  sys_return_val_if_fail (HNODE_CHECK(descendant), false);

  while (descendant) {
    if (descendant->parent == hnode)
      return true;

    descendant = descendant->parent;
  }

  return false;
}

SysUInt sys_hnode_depth (SysHNode *hnode) {
  SysUInt depth = 0;

  while (hnode) {
    depth++;
    hnode = hnode->parent;
  }

  return depth;
}

void sys_hnode_reverse_children (SysHNode *hnode) {
  SysHNode *child;
  SysHNode *last;

  sys_return_if_fail (HNODE_CHECK(hnode));

  child = hnode->children;
  last = NULL;
  while (child) {
    last = child;
    child = last->next;
    last->next = last->prev;
    last->prev = child;
  }
  hnode->children = last;
}

SysUInt sys_hnode_max_height (SysHNode *root) {
  SysHNode *child;
  SysUInt max_height = 0;

  if (!root)
    return 0;

  child = root->children;
  while (child) {
    SysUInt tmp_height;

    tmp_height = sys_hnode_max_height (child);
    if (tmp_height > max_height)
      max_height = tmp_height;
    child = child->next;
  }

  return max_height + 1;
}

static SysBool sys_hnode_traverse_pre_order (SysHNode     *hnode,
    SysTraverseFlags    flags,
    SysHNodeTraverseFunc func,
    SysPointer      user_data) {
  if (hnode->children) {
    SysHNode *child;

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (hnode, user_data))
      return true;

    child = hnode->children;
    while (child) {
      SysHNode *current;

      current = child;
      child = current->next;
      if (sys_hnode_traverse_pre_order (current, flags, func, user_data))
        return true;
    }
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (hnode, user_data))
    return true;

  return false;
}

static SysBool sys_hnode_depth_traverse_pre_order (SysHNode    *hnode,
    SysTraverseFlags    flags,
    SysUInt     depth,
    SysHNodeTraverseFunc func,
    SysPointer    user_data) {
  if (hnode->children) {
    SysHNode *child;

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (hnode, user_data))
      return true;

    depth--;
    if (!depth)
      return false;

    child = hnode->children;
    while (child) {
      SysHNode *current;

      current = child;
      child = current->next;
      if (sys_hnode_depth_traverse_pre_order (current, flags, depth, func, user_data))
        return true;
    }
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (hnode, user_data))
    return true;

  return false;
}

static SysBool sys_hnode_traverse_post_order (SysHNode      *hnode,
    SysTraverseFlags    flags,
    SysHNodeTraverseFunc func,
    SysPointer       user_data) {
  if (hnode->children) {
    SysHNode *child;

    child = hnode->children;
    while (child) {
      SysHNode *current;

      current = child;
      child = current->next;
      if (sys_hnode_traverse_post_order (current, flags, func, user_data))
        return true;
    }

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (hnode, user_data))
      return true;

  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (hnode, user_data))
    return true;

  return false;
}

static SysBool sys_hnode_depth_traverse_post_order (SysHNode     *hnode,
    SysTraverseFlags    flags,
    SysUInt      depth,
    SysHNodeTraverseFunc func,
    SysPointer     user_data) {
  if (hnode->children) {
    depth--;
    if (depth) {
      SysHNode *child;

      child = hnode->children;
      while (child) {
        SysHNode *current;

        current = child;
        child = current->next;
        if (sys_hnode_depth_traverse_post_order (current, flags, depth, func, user_data))
          return true;
      }
    }

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (hnode, user_data))
      return true;

  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (hnode, user_data))
    return true;

  return false;
}

static SysBool sys_hnode_traverse_in_order (SysHNode     *hnode,
    SysTraverseFlags    flags,
    SysHNodeTraverseFunc func,
    SysPointer     user_data) {
  if (hnode->children) {
    SysHNode *child;
    SysHNode *current;

    child = hnode->children;
    current = child;
    child = current->next;

    if (sys_hnode_traverse_in_order (current, flags, func, user_data))
      return true;

    if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (hnode, user_data))
      return true;

    while (child) {
      current = child;
      child = current->next;
      if (sys_hnode_traverse_in_order (current, flags, func, user_data))
        return true;
    }
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (hnode, user_data))
    return true;

  return false;
}

static SysBool sys_hnode_depth_traverse_in_order (SysHNode   *hnode,
    SysTraverseFlags   flags,
    SysUInt    depth,
    SysHNodeTraverseFunc func,
    SysPointer   user_data) {
  if (hnode->children) {
    depth--;
    if (depth) {
      SysHNode *child;
      SysHNode *current;

      child = hnode->children;
      current = child;
      child = current->next;

      if (sys_hnode_depth_traverse_in_order (current, flags, depth, func, user_data))
        return true;

      if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
          func (hnode, user_data))
        return true;

      while (child) {
        current = child;
        child = current->next;
        if (sys_hnode_depth_traverse_in_order (current, flags, depth, func, user_data))
          return true;
      }
    } else if ((flags & SYS_TRAVERSE_NON_LEAFS) &&
        func (hnode, user_data))
      return true;
  } else if ((flags & SYS_TRAVERSE_LEAFS) &&
      func (hnode, user_data))
    return true;

  return false;
}

static SysBool sys_hnode_traverse_level (SysHNode   *hnode,
    SysTraverseFlags   flags,
    SysUInt    level,
    SysHNodeTraverseFunc  func,
    SysPointer           user_data,
    SysBool          *more_levels) {
  if (level == 0)  {
    if (hnode->children) {
      *more_levels = true;
      return (flags & SYS_TRAVERSE_NON_LEAFS) && func (hnode, user_data);
    } else {
      return (flags & SYS_TRAVERSE_LEAFS) && func (hnode, user_data);
    }
  } else  {
    hnode = hnode->children;

    while (hnode) {
      if (sys_hnode_traverse_level (hnode, flags, level - 1, func, user_data, more_levels))
        return true;

      hnode = hnode->next;
    }
  }

  return false;
}

static SysBool sys_hnode_depth_traverse_level (SysHNode             *hnode,
    SysTraverseFlags flags,
    SysInt  depth,
    SysHNodeTraverseFunc  func,
    SysPointer         user_data) {
  SysUInt level;
  SysBool more_levels;

  level = 0;
  while (depth < 0 || level != (SysUInt) depth) {
    more_levels = false;
    if (sys_hnode_traverse_level (hnode, flags, level, func, user_data, &more_levels))
      return true;
    if (!more_levels)
      break;
    level++;
  }
  return false;
}

void sys_hnode_traverse (SysHNode    *root,
    SysTraverseType    order,
    SysTraverseFlags    flags,
    SysInt     depth,
    SysHNodeTraverseFunc func,
    SysPointer    user_data) {
  sys_return_if_fail (root != NULL);
  sys_return_if_fail (func != NULL);
  sys_return_if_fail (order <= SYS_LEVEL_ORDER);
  sys_return_if_fail (flags <= SYS_TRAVERSE_MASK);
  sys_return_if_fail (depth == -1 || depth > 0);

  switch (order) {
    case SYS_PRE_ORDER:
      if (depth < 0)
        sys_hnode_traverse_pre_order (root, flags, func, user_data);
      else
        sys_hnode_depth_traverse_pre_order (root, flags, depth, func, user_data);
      break;
    case SYS_POST_ORDER:
      if (depth < 0)
        sys_hnode_traverse_post_order (root, flags, func, user_data);
      else
        sys_hnode_depth_traverse_post_order (root, flags, depth, func, user_data);
      break;
    case SYS_IN_ORDER:
      if (depth < 0)
        sys_hnode_traverse_in_order (root, flags, func, user_data);
      else
        sys_hnode_depth_traverse_in_order (root, flags, depth, func, user_data);
      break;
    case SYS_LEVEL_ORDER:
      sys_hnode_depth_traverse_level (root, flags, depth, func, user_data);
      break;
  }
}

static void sys_hnode_count_func (SysHNode  *hnode,
    SysTraverseFlags flags,
    SysUInt  *n) {
  if (hnode->children) {
    SysHNode *child;

    if (flags & SYS_TRAVERSE_NON_LEAFS)
      (*n)++;

    child = hnode->children;
    while (child) {
      sys_hnode_count_func (child, flags, n);
      child = child->next;
    }
  } else if (flags & SYS_TRAVERSE_LEAFS)
    (*n)++;
}

SysUInt sys_hnode_n_hnodes (SysHNode        *root,
    SysTraverseFlags  flags) {
  SysUInt n = 0;

  sys_return_val_if_fail (root != NULL, 0);
  sys_return_val_if_fail (flags <= SYS_TRAVERSE_MASK, 0);

  sys_hnode_count_func (root, flags, &n);

  return n;
}

SysHNode* sys_hnode_last_child (SysHNode *hnode) {
  sys_return_val_if_fail (hnode != NULL, NULL);

  hnode = hnode->children;
  if (hnode)
    while (hnode->next)
      hnode = hnode->next;

  return hnode;
}

SysHNode* sys_hnode_nth_child (SysHNode *hnode,
    SysUInt  n) {
  sys_return_val_if_fail (hnode != NULL, NULL);

  hnode = hnode->children;
  if (hnode)
    while ((n-- > 0) && hnode)
      hnode = hnode->next;

  return hnode;
}

SysUInt sys_hnode_n_children (SysHNode *hnode) {
  SysUInt n = 0;

  sys_return_val_if_fail (hnode != NULL, 0);

  hnode = hnode->children;
  while (hnode) {
    n++;
    hnode = hnode->next;
  }

  return n;
}

SysInt sys_hnode_child_position (SysHNode *hnode,
    SysHNode *child) {
  SysUInt n = 0;

  sys_return_val_if_fail (hnode != NULL, -1);
  sys_return_val_if_fail (child != NULL, -1);
  sys_return_val_if_fail (child->parent == hnode, -1);

  hnode = hnode->children;
  while (hnode) {
    if (hnode == child)
      return n;
    n++;
    hnode = hnode->next;
  }

  return -1;
}

SysHNode* sys_hnode_first_sibling (SysHNode *hnode) {
  sys_return_val_if_fail (hnode != NULL, NULL);

  if (hnode->parent)
    return hnode->parent->children;

  while (hnode->prev)
    hnode = hnode->prev;

  return hnode;
}

SysHNode* sys_hnode_last_sibling (SysHNode *hnode) {
  sys_return_val_if_fail (hnode != NULL, NULL);

  while (hnode->next)
    hnode = hnode->next;

  return hnode;
}

void sys_hnode_children_foreach (SysHNode    *hnode,
    SysTraverseFlags    flags,
    SysHNodeForeachFunc  func,
    SysPointer    user_data) {
  sys_return_if_fail (hnode != NULL);
  sys_return_if_fail (flags <= SYS_TRAVERSE_MASK);
  sys_return_if_fail (func != NULL);

  hnode = hnode->children;
  while (hnode) {
    SysHNode *current;

    current = hnode;
    hnode = current->next;
    if (SYS_HNODE_IS_LEAF (current)) {
      if (flags & SYS_TRAVERSE_LEAFS)
        func (current, user_data);
    } else {
      if (flags & SYS_TRAVERSE_NON_LEAFS)
        func (current, user_data);
    }
  }
}

void sys_hnode_handle_bfs_r(SysHNode *self, SysHNodeFunc func, SysPointer user_data) {
  sys_return_if_fail(HNODE_CHECK(self));

  SysHNode* node;
  SysHNode* nnode;
  SysQueue* queue = sys_queue_new();

  sys_queue_push_head(queue, (SysPointer)self);

  while (!sys_queue_is_empty(queue)) {
    node = sys_queue_pop_tail(queue);

    if(!func(node, user_data)) {
      break;
    }

    for(nnode = node->children; nnode; nnode = nnode->next) {

      sys_queue_push_head(queue, nnode);
    }
  }

  sys_queue_free(queue);
}

void sys_hnode_handle_ff_r(SysHNode *self, SysHNodeFunc func, SysPointer user_data) {
  sys_return_if_fail(HNODE_CHECK(self));

  func(self, user_data);

  if (self->children) {

    sys_hnode_handle_ff_r(self->children, func, user_data);
  }

  if (self->next) {

    sys_hnode_handle_ff_r(self->next, func, user_data);
  }
}

void sys_hnode_handle_ft_r(SysHNode *self, SysHNodeFunc func, SysPointer user_data) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(HNODE_CHECK(self));

  if (self->children) {

    sys_hnode_handle_ft_r(self->children, func, user_data);
  }

  if (self->next) {

    sys_hnode_handle_ft_r(self->next, func, user_data);
  }

  func(self, user_data);
}

SysHNode* sys_hnode_append(SysHNode *parent, SysHNode *node) {
  sys_return_val_if_fail(HNODE_CHECK(parent), NULL);
  sys_return_val_if_fail(HNODE_CHECK(node), NULL);

  SysHNode *last_child = sys_hnode_get_last_child(parent);
  sys_hnode_insert_after(parent, last_child, node);

  sys_hnode_set_last_child(parent, node);

  return node;
}

void sys_hnode_set_last_child(SysHNode *self, SysHNode * last_child) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(HNODE_CHECK(self));

  self->last_child = last_child;
}

SysHNode * sys_hnode_get_last_child(SysHNode *self) {
  sys_return_val_if_fail(self != NULL, NULL);
  sys_return_val_if_fail(HNODE_CHECK(self), NULL);

  return self->last_child;
}

SysHNode* sys_hnode_children(SysHNode* self) {
  if (self == NULL) { return NULL; }
  sys_return_val_if_fail(HNODE_CHECK(self), NULL);

  return self->children;
}

SysHNode* sys_hnode_next(SysHNode* self) {
  sys_return_val_if_fail(HNODE_CHECK(self), NULL);

  return self->next;
}

SysHNode* sys_hnode_prev(SysHNode* self) {
  sys_return_val_if_fail(HNODE_CHECK(self), NULL);

  return self->prev;
}

void sys_hnode_set_parent(SysHNode *self, SysHNode *parent) {
  sys_return_if_fail(HNODE_CHECK(self));
  sys_return_if_fail(HNODE_CHECK(parent));

  self->parent = parent;
}

SysHNode* sys_hnode_parent(SysHNode* self) {
  sys_return_val_if_fail(HNODE_CHECK(self), NULL);

  return self->parent;
}

SysBool sys_hnode_has_one_child(SysHNode *self) {
  sys_return_val_if_fail(HNODE_CHECK(self), false);

  return self->children != NULL && self->next == NULL;
}

