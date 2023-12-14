/**
 * this code from glib GTree
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */
#include <System/DataTypes/SysTree.h>

#define MAX_SYSTREE_HEIGHT 40

struct _SysTree {
  SysTreeNode        *root;
  SysCompareDataFunc  key_compare;
  SysDestroyFunc    key_destroy_func;
  SysDestroyFunc    value_destroy_func;
  SysPointer          key_compare_data;
  SysUInt             nnodes;
  SysInt              ref_count;
};

struct _SysTreeNode {
  SysPointer   key;
  SysPointer   value;
  SysTreeNode *left;
  SysTreeNode *right;
  SysInt8      balance;
  SysUInt8     left_child;
  SysUInt8     right_child;
};


static SysTreeNode* sys_tree_node_new                   (SysPointer       key,
                                                     SysPointer       value);
static SysTreeNode *sys_tree_insert_internal (SysTree *tree,
                                          SysPointer key,
                                          SysPointer value,
                                          SysBool replace);
static SysBool   sys_tree_remove_internal            (SysTree         *tree,
                                                     const SysPointer  key,
                                                     SysBool       steal);
static SysTreeNode* sys_tree_node_balance               (SysTreeNode     *node);
static SysTreeNode *sys_tree_find_node                  (SysTree         *tree,
                                                     const SysPointer  key);
static SysInt       sys_tree_node_pre_order             (SysTreeNode     *node,
                                                     SysTraverseFunc  traverse_func,
                                                     SysPointer       data);
static SysInt       sys_tree_node_in_order              (SysTreeNode     *node,
                                                     SysTraverseFunc  traverse_func,
                                                     SysPointer       data);
static SysInt       sys_tree_node_post_order            (SysTreeNode     *node,
                                                     SysTraverseFunc  traverse_func,
                                                     SysPointer       data);
static SysTreeNode *sys_tree_node_search (SysTreeNode *node,
                                      SysCompareFunc search_func,
                                      const SysPointer data);
static SysTreeNode* sys_tree_node_rotate_left           (SysTreeNode     *node);
static SysTreeNode* sys_tree_node_rotate_right          (SysTreeNode     *node);

static SysTreeNode* sys_tree_node_new (SysPointer key,
                 SysPointer value) {
  SysTreeNode *node = sys_slice_new (SysTreeNode);

  node->balance = 0;
  node->left = NULL;
  node->right = NULL;
  node->left_child = false;
  node->right_child = false;
  node->key = key;
  node->value = value;

  return node;
}

SysTree* sys_tree_new (SysCompareFunc key_compare_func) {
  sys_return_val_if_fail (key_compare_func != NULL, NULL);

  return sys_tree_new_full ((SysCompareDataFunc) key_compare_func, NULL,
                          NULL, NULL);
}

SysTree* sys_tree_new_with_data (SysCompareDataFunc key_compare_func,
                      SysPointer         key_compare_data) {
  sys_return_val_if_fail (key_compare_func != NULL, NULL);
  
  return sys_tree_new_full (key_compare_func, key_compare_data, 
                          NULL, NULL);
}

SysTree* sys_tree_new_full (SysCompareDataFunc key_compare_func,
                 SysPointer         key_compare_data,
                 SysDestroyFunc   key_destroy_func,
                 SysDestroyFunc   value_destroy_func) {
  SysTree *tree;
  
  sys_return_val_if_fail (key_compare_func != NULL, NULL);
  
  tree = sys_slice_new (SysTree);
  tree->root               = NULL;
  tree->key_compare        = key_compare_func;
  tree->key_destroy_func   = key_destroy_func;
  tree->value_destroy_func = value_destroy_func;
  tree->key_compare_data   = key_compare_data;
  tree->nnodes             = 0;
  tree->ref_count          = 1;
  
  return tree;
}

SysTreeNode* sys_tree_node_first (SysTree *tree) {
  SysTreeNode *tmp;

  sys_return_val_if_fail (tree != NULL, NULL);

  if (!tree->root)
    return NULL;

  tmp = tree->root;

  while (tmp->left_child)
    tmp = tmp->left;

  return tmp;
}

/**
 * sys_tree_node_last:
 * @tree: a #SysTree *  *
 * Returns the last in-order node of the tree, or %NULL
 * for an empty tree.
 *
 * Returns: (nullable) (transfer none): the last node in the tree
 *
 * Since: 2.68
 */
SysTreeNode* sys_tree_node_last (SysTree *tree) {
  SysTreeNode *tmp;

  sys_return_val_if_fail (tree != NULL, NULL);

  if (!tree->root)
    return NULL;

  tmp = tree->root;

  while (tmp->right_child)
    tmp = tmp->right;

  return tmp;
}

/**
 * sys_tree_node_previous
 * @node: a #SysTree node
 *
 * Returns the previous in-order node of the tree, or %NULL
 * if the passed node was already the first one.
 *
 * Returns: (nullable) (transfer none): the previous node in the tree
 *
 * Since: 2.68
 */
SysTreeNode* sys_tree_node_previous (SysTreeNode *node) {
  SysTreeNode *tmp;

  sys_return_val_if_fail (node != NULL, NULL);

  tmp = node->left;

  if (node->left_child)
    while (tmp->right_child)
      tmp = tmp->right;

  return tmp;
}

/**
 * sys_tree_node_next
 * @node: a #SysTree node
 *
 * Returns the next in-order node of the tree, or %NULL
 * if the passed node was already the last one.
 *
 * Returns: (nullable) (transfer none): the next node in the tree
 *
 * Since: 2.68
 */
SysTreeNode* sys_tree_node_next (SysTreeNode *node) {
  SysTreeNode *tmp;

  sys_return_val_if_fail (node != NULL, NULL);

  tmp = node->right;

  if (node->right_child)
    while (tmp->left_child)
      tmp = tmp->left;

  return tmp;
}

void sys_tree_remove_all (SysTree *tree) {
  SysTreeNode *node;
  SysTreeNode *next;

  sys_return_if_fail (tree != NULL);

  node = sys_tree_node_first (tree);

  while (node) {
      next = sys_tree_node_next (node);

      if (tree->key_destroy_func)
        tree->key_destroy_func (node->key);
      if (tree->value_destroy_func)
        tree->value_destroy_func (node->value);
      sys_slice_free (SysTreeNode, node);

#ifdef SYS_TREE_DEBUG
      sys_assert (tree->nnodes > 0);
      tree->nnodes--;
#endif

      node = next;
    }

#ifdef SYS_TREE_DEBUG
  sys_assert (tree->nnodes == 0);
#endif

  tree->root = NULL;
#ifndef SYS_TREE_DEBUG
  tree->nnodes = 0;
#endif
}

SysTree* sys_tree_ref (SysTree *tree) {
  sys_return_val_if_fail (tree != NULL, NULL);

  sys_atomic_int_inc (&tree->ref_count);

  return tree;
}

void sys_tree_unref (SysTree *tree) {
  sys_return_if_fail (tree != NULL);

  if (sys_atomic_int_dec_and_test (&tree->ref_count)) {
      sys_tree_remove_all (tree);
      sys_slice_free (SysTree, tree);
    }
}

void sys_tree_destroy (SysTree *tree) {
  sys_return_if_fail (tree != NULL);

  sys_tree_remove_all (tree);
  sys_tree_unref (tree);
}

SysTreeNode* sys_tree_insert_node (SysTree    *tree,
                    SysPointer  key,
                    SysPointer  value) {
  SysTreeNode *node;

  sys_return_val_if_fail (tree != NULL, NULL);

  node = sys_tree_insert_internal (tree, key, value, false);

  return node;
}

void sys_tree_insert (SysTree    *tree,
               SysPointer  key,
               SysPointer  value) {
  sys_tree_insert_node (tree, key, value);
}

SysTreeNode* sys_tree_replace_node (SysTree    *tree,
                     SysPointer  key,
                     SysPointer  value) {
  SysTreeNode *node;

  sys_return_val_if_fail (tree != NULL, NULL);

  node = sys_tree_insert_internal (tree, key, value, true);

#ifdef SYS_TREE_DEBUG
  sys_tree_node_check (tree->root);
#endif

  return node;
}

void sys_tree_replace (SysTree    *tree,
                SysPointer  key,
                SysPointer  value) {
  sys_tree_replace_node (tree, key, value);
}

/* internal insert routine */
static SysTreeNode* sys_tree_insert_internal (SysTree    *tree,
                        SysPointer  key,
                        SysPointer  value,
                        SysBool  replace) {
  SysTreeNode *node, *retnode;
  SysTreeNode *path[MAX_SYSTREE_HEIGHT];
  int idx;

  sys_return_val_if_fail (tree != NULL, NULL);

  if (!tree->root) {
      tree->root = sys_tree_node_new (key, value);
      tree->nnodes++;
      return tree->root;
    }

  idx = 0;
  path[idx++] = NULL;
  node = tree->root;

  while (1) {
      int cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      
      if (cmp == 0) {
          if (tree->value_destroy_func)
            tree->value_destroy_func (node->value);

          node->value = value;

          if (replace) {
              if (tree->key_destroy_func)
                tree->key_destroy_func (node->key);

              node->key = key;
            }
          else {
              /* free the passed key */
              if (tree->key_destroy_func)
                tree->key_destroy_func (key);
            }

          return node;
        }
      else if (cmp < 0) {
          if (node->left_child) {
              path[idx++] = node;
              node = node->left;
            }
          else {
              SysTreeNode *child = sys_tree_node_new (key, value);

              child->left = node->left;
              child->right = node;
              node->left = child;
              node->left_child = true;
              node->balance -= 1;

              tree->nnodes++;

              retnode = child;
              break;
            }
        }
      else {
          if (node->right_child) {
              path[idx++] = node;
              node = node->right;
            }
          else {
              SysTreeNode *child = sys_tree_node_new (key, value);

              child->right = node->right;
              child->left = node;
              node->right = child;
              node->right_child = true;
              node->balance += 1;

              tree->nnodes++;

              retnode = child;
              break;
            }
        }
    }

  /* Restore balance. This is the goodness of a non-recursive
   * implementation, when we are done with balancing we 'break'
   * the loop and we are done.
   */
  while (1) {
      SysTreeNode *bparent = path[--idx];
      SysBool left_node = (bparent && node == bparent->left);
      sys_assert (!bparent || bparent->left == node || bparent->right == node);

      if (node->balance < -1 || node->balance > 1) {
          node = sys_tree_node_balance (node);
          if (bparent == NULL)
            tree->root = node;
          else if (left_node)
            bparent->left = node;
          else
            bparent->right = node;
        }

      if (node->balance == 0 || bparent == NULL)
        break;
      
      if (left_node)
        bparent->balance -= 1;
      else
        bparent->balance += 1;

      node = bparent;
    }

  return retnode;
}

SysBool sys_tree_remove (SysTree         *tree,
               const SysPointer  key) {
  SysBool removed;

  sys_return_val_if_fail (tree != NULL, false);

  removed = sys_tree_remove_internal (tree, key, false);

#ifdef SYS_TREE_DEBUG
  sys_tree_node_check (tree->root);
#endif

  return removed;
}

SysBool sys_tree_steal (SysTree         *tree,
              const SysPointer  key) {
  SysBool removed;

  sys_return_val_if_fail (tree != NULL, false);

  removed = sys_tree_remove_internal (tree, key, true);

#ifdef SYS_TREE_DEBUG
  sys_tree_node_check (tree->root);
#endif

  return removed;
}

static SysBool sys_tree_remove_internal (SysTree         *tree,
                        const SysPointer  key,
                        SysBool       steal) {
  SysTreeNode *node, *parent, *balance;
  SysTreeNode *path[MAX_SYSTREE_HEIGHT];
  int idx;
  SysBool left_node;

  sys_return_val_if_fail (tree != NULL, false);

  if (!tree->root)
    return false;

  idx = 0;
  path[idx++] = NULL;
  node = tree->root;

  while (1) {
      int cmp = tree->key_compare (key, node->key, tree->key_compare_data);

      if (cmp == 0)
        break;
      else if (cmp < 0) {
          if (!node->left_child)
            return false;

          path[idx++] = node;
          node = node->left;
        }
      else {
          if (!node->right_child)
            return false;

          path[idx++] = node;
          node = node->right;
        }
    }

  /* The following code is almost equal to sys_tree_remove_node,
   * except that we do not have to call sys_tree_node_parent.
   */
  balance = parent = path[--idx];
  sys_assert (!parent || parent->left == node || parent->right == node);
  left_node = (parent && node == parent->left);

  if (!node->left_child) {
      if (!node->right_child) {
          if (!parent)
            tree->root = NULL;
          else if (left_node) {
              parent->left_child = false;
              parent->left = node->left;
              parent->balance += 1;
            }
          else {
              parent->right_child = false;
              parent->right = node->right;
              parent->balance -= 1;
            }
        }
      else /* node has a right child */ {
          SysTreeNode *tmp = sys_tree_node_next (node);
          tmp->left = node->left;

          if (!parent)
            tree->root = node->right;
          else if (left_node) {
              parent->left = node->right;
              parent->balance += 1;
            }
          else {
              parent->right = node->right;
              parent->balance -= 1;
            }
        }
    }
  else /* node has a left child */ {
      if (!node->right_child) {
          SysTreeNode *tmp = sys_tree_node_previous (node);
          tmp->right = node->right;

          if (parent == NULL)
            tree->root = node->left;
          else if (left_node) {
              parent->left = node->left;
              parent->balance += 1;
            }
          else {
              parent->right = node->left;
              parent->balance -= 1;
            }
        }
      else /* node has a both children (pant, pant!) */ {
          SysTreeNode *prev = node->left;
          SysTreeNode *next = node->right;
          SysTreeNode *nextp = node;
          int old_idx = idx + 1;
          idx++;

          /* path[idx] == parent */
          /* find the immediately next node (and its parent) */
          while (next->left_child) {
              path[++idx] = nextp = next;
              next = next->left;
            }

          path[old_idx] = next;
          balance = path[idx];

          /* remove 'next' from the tree */
          if (nextp != node) {
              if (next->right_child)
                nextp->left = next->right;
              else
                nextp->left_child = false;
              nextp->balance += 1;

              next->right_child = true;
              next->right = node->right;
            }
          else
            node->balance -= 1;

          /* set the prev to point to the right place */
          while (prev->right_child)
            prev = prev->right;
          prev->right = next;

          /* prepare 'next' to replace 'node' */
          next->left_child = true;
          next->left = node->left;
          next->balance = node->balance;

          if (!parent)
            tree->root = next;
          else if (left_node)
            parent->left = next;
          else
            parent->right = next;
        }
    }

  /* restore balance */
  if (balance)
    while (1) {
        SysTreeNode *bparent = path[--idx];
        sys_assert (!bparent || bparent->left == balance || bparent->right == balance);
        left_node = (bparent && balance == bparent->left);

        if(balance->balance < -1 || balance->balance > 1) {
            balance = sys_tree_node_balance (balance);
            if (!bparent)
              tree->root = balance;
            else if (left_node)
              bparent->left = balance;
            else
              bparent->right = balance;
          }

        if (balance->balance != 0 || !bparent)
          break;

        if (left_node)
          bparent->balance += 1;
        else
          bparent->balance -= 1;

        balance = bparent;
      }

  if (!steal) {
      if (tree->key_destroy_func)
        tree->key_destroy_func (node->key);
      if (tree->value_destroy_func)
        tree->value_destroy_func (node->value);
    }

  sys_slice_free (SysTreeNode, node);

  tree->nnodes--;

  return true;
}

SysPointer sys_tree_node_key (SysTreeNode *node) {
  sys_return_val_if_fail (node != NULL, NULL);

  return node->key;
}

SysPointer sys_tree_node_value (SysTreeNode *node) {
  sys_return_val_if_fail (node != NULL, NULL);

  return node->value;
}

SysTreeNode* sys_tree_lookup_node (SysTree         *tree,
                    const SysPointer  key) {
  sys_return_val_if_fail (tree != NULL, NULL);

  return sys_tree_find_node (tree, key);
}

SysPointer sys_tree_lookup (SysTree         *tree,
               const SysPointer  key) {
  SysTreeNode *node;

  node = sys_tree_lookup_node (tree, key);

  return node ? node->value : NULL;
}

SysBool sys_tree_lookup_extended (SysTree         *tree,
                        const SysPointer  lookup_key,
                        SysPointer      *orisys_key,
                        SysPointer      *value) {
  SysTreeNode *node;
  
  sys_return_val_if_fail (tree != NULL, false);
  
  node = sys_tree_find_node (tree, lookup_key);
  
  if (node) {
      if (orisys_key)
        *orisys_key = node->key;
      if (value)
        *value = node->value;
      return true;
    }
  else
    return false;
}

void sys_tree_foreach (SysTree         *tree,
                SysTraverseFunc  func,
                SysPointer       user_data) {
  SysTreeNode *node;

  sys_return_if_fail (tree != NULL);
  
  if (!tree->root)
    return;

  node = sys_tree_node_first (tree);

  while (node) {
      if ((*func) (node->key, node->value, user_data))
        break;
      
      node = sys_tree_node_next (node);
    }
}

void sys_tree_foreach_node (SysTree             *tree,
                     SysTraverseNodeFunc  func,
                     SysPointer           user_data) {
  SysTreeNode *node;

  sys_return_if_fail (tree != NULL);

  if (!tree->root)
    return;

  node = sys_tree_node_first (tree);

  while (node) {
      if ((*func) (node, user_data))
        break;

      node = sys_tree_node_next (node);
    }
}

void sys_tree_traverse (SysTree         *tree,
                 SysTraverseFunc  traverse_func,
                 SysTraverseType  traverse_type,
                 SysPointer       user_data) {
  sys_return_if_fail (tree != NULL);

  if (!tree->root)
    return;

  switch (traverse_type) {
    case SYS_PRE_ORDER:
      sys_tree_node_pre_order (tree->root, traverse_func, user_data);
      break;

    case SYS_IN_ORDER:
      sys_tree_node_in_order (tree->root, traverse_func, user_data);
      break;

    case SYS_POST_ORDER:
      sys_tree_node_post_order (tree->root, traverse_func, user_data);
      break;

    case SYS_LEVEL_ORDER:
      sys_warning_N("%s", "sys_tree_traverse(): traverse type SYS_LEVEL_ORDER isn't implemented.");
      break;
    }
}

SysTreeNode* sys_tree_search_node (SysTree         *tree,
                    SysCompareFunc   search_func,
                    const SysPointer  user_data) {
  sys_return_val_if_fail (tree != NULL, NULL);

  if (!tree->root)
    return NULL;

  return sys_tree_node_search (tree->root, search_func, user_data);
}

SysPointer sys_tree_search (SysTree         *tree,
               SysCompareFunc   search_func,
               const SysPointer  user_data) {
  SysTreeNode *node;

  node = sys_tree_search_node (tree, search_func, user_data);

  return node ? node->value : NULL;
}

SysTreeNode* sys_tree_lower_bound (SysTree         *tree,
                    const SysPointer  key) {
  SysTreeNode *node, *result;
  SysInt cmp;

  sys_return_val_if_fail (tree != NULL, NULL);

  node = tree->root;
  if (!node)
    return NULL;

  result = NULL;
  while (1) {
      cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      if (cmp <= 0) {
          result = node;

          if (!node->left_child)
            return result;

          node = node->left;
        }
      else {
          if (!node->right_child)
            return result;

          node = node->right;
        }
    }
}

SysTreeNode* sys_tree_upper_bound (SysTree         *tree,
                    const SysPointer  key) {
  SysTreeNode *node, *result;
  SysInt cmp;

  sys_return_val_if_fail (tree != NULL, NULL);

  node = tree->root;
  if (!node)
    return NULL;

  result = NULL;
  while (1) {
      cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      if (cmp < 0) {
          result = node;

          if (!node->left_child)
            return result;

          node = node->left;
        }
      else {
          if (!node->right_child)
            return result;

          node = node->right;
        }
    }
}

SysInt sys_tree_height (SysTree *tree) {
  SysTreeNode *node;
  SysInt height;

  sys_return_val_if_fail (tree != NULL, 0);

  if (!tree->root)
    return 0;

  height = 0;
  node = tree->root;

  while (1) {
      height += 1 + max(node->balance, 0);

      if (!node->left_child)
        return height;
      
      node = node->left;
    }
}

SysInt sys_tree_nnodes (SysTree *tree) {
  sys_return_val_if_fail (tree != NULL, 0);

  return tree->nnodes;
}

static SysTreeNode* sys_tree_node_balance (SysTreeNode *node) {
  if (node->balance < -1) {
      if (node->left->balance > 0)
        node->left = sys_tree_node_rotate_left (node->left);
      node = sys_tree_node_rotate_right (node);
    }
  else if (node->balance > 1) {
      if (node->right->balance < 0)
        node->right = sys_tree_node_rotate_right (node->right);
      node = sys_tree_node_rotate_left (node);
    }

  return node;
}

static SysTreeNode* sys_tree_find_node (SysTree        *tree,
                  const SysPointer key) {
  SysTreeNode *node;
  SysInt cmp;

  node = tree->root;
  if (!node)
    return NULL;

  while (1) {
      cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      if (cmp == 0)
        return node;
      else if (cmp < 0) {
          if (!node->left_child)
            return NULL;

          node = node->left;
        }
      else {
          if (!node->right_child)
            return NULL;

          node = node->right;
        }
    }
}

static SysInt sys_tree_node_pre_order (SysTreeNode     *node,
                       SysTraverseFunc  traverse_func,
                       SysPointer       data) {
  if ((*traverse_func) (node->key, node->value, data))
    return true;

  if (node->left_child) {
      if (sys_tree_node_pre_order (node->left, traverse_func, data))
        return true;
    }

  if (node->right_child) {
      if (sys_tree_node_pre_order (node->right, traverse_func, data))
        return true;
    }

  return false;
}

static SysInt sys_tree_node_in_order (SysTreeNode     *node,
                      SysTraverseFunc  traverse_func,
                      SysPointer       data) {
  if (node->left_child) {
      if (sys_tree_node_in_order (node->left, traverse_func, data))
        return true;
    }

  if ((*traverse_func) (node->key, node->value, data))
    return true;

  if (node->right_child) {
      if (sys_tree_node_in_order (node->right, traverse_func, data))
        return true;
    }
  
  return false;
}

static SysInt sys_tree_node_post_order (SysTreeNode     *node,
                        SysTraverseFunc  traverse_func,
                        SysPointer       data) {
  if (node->left_child) {
      if (sys_tree_node_post_order (node->left, traverse_func, data))
        return true;
    }

  if (node->right_child) {
      if (sys_tree_node_post_order (node->right, traverse_func, data))
        return true;
    }

  if ((*traverse_func) (node->key, node->value, data))
    return true;

  return false;
}

static SysTreeNode* sys_tree_node_search (SysTreeNode     *node,
                    SysCompareFunc   search_func,
                    const SysPointer  data) {
  SysInt dir;

  if (!node)
    return NULL;

  while (1)  {
      dir = (* search_func) (node->key, data);
      if (dir == 0)
        return node;
      else if (dir < 0)  {
          if (!node->left_child)
            return NULL;

          node = node->left;
        }
      else {
          if (!node->right_child)
            return NULL;

          node = node->right;
        }
    }
}

static SysTreeNode* sys_tree_node_rotate_left (SysTreeNode *node) {
  SysTreeNode *right;
  SysInt a_bal;
  SysInt b_bal;

  right = node->right;

  if (right->left_child)
    node->right = right->left;
  else {
      node->right_child = false;
      right->left_child = true;
    }
  right->left = node;

  a_bal = node->balance;
  b_bal = right->balance;

  if (b_bal <= 0) {
      if (a_bal >= 1)
        right->balance = (SysInt8)(b_bal - 1);
      else
        right->balance = (SysInt8)(a_bal + b_bal - 2);
      node->balance = (SysInt8)(a_bal - 1);
    }
  else {
      if (a_bal <= b_bal)
        right->balance = (SysInt8)a_bal - 2;
      else
        right->balance = (SysInt8)b_bal - 1;
      node->balance = (SysInt8)(a_bal - b_bal - 1);
    }

  return right;
}

static SysTreeNode* sys_tree_node_rotate_right (SysTreeNode *node) {
  SysTreeNode *left;
  SysInt a_bal;
  SysInt b_bal;

  left = node->left;

  if (left->right_child) {
    node->left = left->right;

  } else {
    node->left_child = false;
    left->right_child = true;
  }
  left->right = node;

  a_bal = node->balance;
  b_bal = left->balance;

  if (b_bal <= 0) {
      if (b_bal > a_bal)
        left->balance = (SysInt8)(b_bal + 1);
      else
        left->balance = (SysInt8)a_bal + 2;
      node->balance = (SysInt8)(a_bal - b_bal + 1);
    }
  else {
      if (a_bal <= -1)
        left->balance = (SysInt8)b_bal + 1;
      else
        left->balance = (SysInt8)(a_bal + b_bal + 2);
      node->balance = (SysInt8)a_bal + 1;
    }

  return left;
}
