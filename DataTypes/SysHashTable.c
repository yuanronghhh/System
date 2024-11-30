#include <System/DataTypes/SysHashTable.h>
#include <System/Type/SysBlock.h>
#include <System/Utils/SysStr.h>

/**
 * this code from glib hashtable
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

#define HASH_TABLE_MIN_SHIFT 3 /* 1 << 3 == 8 buckets */

#define UNUSED_HASH_VALUE 0
#define TOMBSTONE_HASH_VALUE 1
#define HASH_IS_UNUSED(h_) ((h_) == UNUSED_HASH_VALUE)
#define HASH_IS_TOMBSTONE(h_) ((h_) == TOMBSTONE_HASH_VALUE)
#define HASH_IS_REAL(h_) ((h_) >= 2)

struct _SysHashTable {
  SysInt size;
  SysInt mod;
  SysUInt mask;
  SysInt nnodes;
  SysInt noccupied; /* nnodes + tombstones */

  SysPointer *keys;
  SysUInt *hashes;
  SysPointer *values;

  SysHashFunc hash_func;
  SysEqualFunc key_equal_func;
  SysRef ref_count;

  SysDestroyFunc key_destroy_func;
  SysDestroyFunc value_destroy_func;
};

typedef struct {
  SysHashTable *hash_table;
  SysPointer dummy1;
  SysPointer dummy2;
  SysInt position;
  SysBool dummy3;
  SysInt version;
} RealIter;

static const SysInt prime_mod[] = {
    1, /* For 1 << 0 */
    2,         3,          7,         13,       31,        61,
    127,       251,        509,       1021,     2039,      4093,
    8191,      16381,      32749,     65521, /* For 1 << 16 */
    131071,    262139,     524287,    1048573,  2097143,   4194301,
    8388593,   16777213,   33554393,  67108859, 134217689, 268435399,
    536870909, 1073741789, 2147483647 /* For 1 << 31 */
};

static void sys_hash_table_set_shift(SysHashTable *hash_table, SysInt shift) {
  SysInt i;
  SysUInt mask = 0;

  hash_table->size = 1 << shift;
  hash_table->mod = prime_mod[shift];

  for (i = 0; i < shift; i++) {
    mask <<= 1;
    mask |= 1;
  }

  hash_table->mask = mask;
}

static SysInt sys_hash_table_find_closest_shift(SysInt n) {
  SysInt i;

  for (i = 0; n; i++)
    n >>= 1;

  return i;
}

static void sys_hash_table_set_shift_from_size(SysHashTable *hash_table,
                                               SysInt size) {
  SysInt shift;

  shift = sys_hash_table_find_closest_shift(size);
  shift = max(shift, HASH_TABLE_MIN_SHIFT);

  sys_hash_table_set_shift(hash_table, shift);
}

static inline SysUInt sys_hash_table_lookup_node(SysHashTable *hash_table,
                                                 const SysPointer key,
                                                 SysUInt *hash_return) {
  SysUInt node_index;
  SysUInt node_hash;
  SysUInt hash_value;
  SysUInt first_tombstone = 0;
  SysBool have_tombstone = false;
  SysUInt step = 0;

  hash_value = hash_table->hash_func(key);
  if (!HASH_IS_REAL(hash_value))
    hash_value = 2;

  *hash_return = hash_value;

  node_index = hash_value % hash_table->mod;
  node_hash = hash_table->hashes[node_index];

  while (!HASH_IS_UNUSED(node_hash)) {
    /* We first check if our full hash values
     * are equal so we can avoid calling the full-blown
     * key equality function in most cases.
     */
    if (node_hash == hash_value) {
      SysPointer node_key = hash_table->keys[node_index];

      if (hash_table->key_equal_func) {
        if (hash_table->key_equal_func(node_key, key))
          return node_index;
      } else if (node_key == key) {
        return node_index;
      }
    } else if (HASH_IS_TOMBSTONE(node_hash) && !have_tombstone) {
      first_tombstone = node_index;
      have_tombstone = true;
    }

    step++;
    node_index += step;
    node_index &= hash_table->mask;
    node_hash = hash_table->hashes[node_index];
  }

  if (have_tombstone)
    return first_tombstone;

  return node_index;
}

static void sys_hash_table_remove_node(SysHashTable *hash_table, SysInt i,
                                       SysBool notify) {
  SysPointer key;
  SysPointer value;

  key = hash_table->keys[i];
  value = hash_table->values[i];

  /* Erect tombstone */
  hash_table->hashes[i] = TOMBSTONE_HASH_VALUE;

  /* Be GC friendly */
  hash_table->keys[i] = NULL;
  hash_table->values[i] = NULL;

  hash_table->nnodes--;

  if (notify && hash_table->key_destroy_func) {
    hash_table->key_destroy_func(key);
  }

  if (notify && hash_table->value_destroy_func) {
    hash_table->value_destroy_func(value);
  }
}

static void sys_hash_table_remove_all_nodes(SysHashTable *hash_table,
                                            SysBool notify, SysBool destruction) {
  SysInt i;
  SysPointer key;
  SysPointer value;
  SysInt old_size;
  SysPointer *old_keys;
  SysPointer *old_values;
  SysUInt *old_hashes;

  /* If the hash table is already empty, there is nothing to be done. */
  if (hash_table->nnodes == 0)
    return;

  hash_table->nnodes = 0;
  hash_table->noccupied = 0;

  if (!notify || (hash_table->key_destroy_func == NULL &&
                  hash_table->value_destroy_func == NULL)) {
    if (!destruction) {
      memset(hash_table->hashes, 0, hash_table->size * sizeof(SysUInt));
      memset(hash_table->keys, 0, hash_table->size * sizeof(SysPointer));
      memset(hash_table->values, 0, hash_table->size * sizeof(SysPointer));
    }

    return;
  }

  /* Keep the old storage space around to iterate over it. */
  old_size = hash_table->size;
  old_keys = hash_table->keys;
  old_values = hash_table->values;
  old_hashes = hash_table->hashes;

  /* Now create a new storage space; If the table is destroyed we can use the
   * shortcut of not creating a new storage. This saves the allocation at the
   * cost of not allowing any recursive access.
   * However, the application doesn't own any reference anymore, so access
   * is not allowed. If accesses are done, then either an sys_return_val_if_fail( or crash
   * *will* happen. */
  sys_hash_table_set_shift(hash_table, HASH_TABLE_MIN_SHIFT);
  if (!destruction) {
    hash_table->keys = sys_new0(SysPointer, hash_table->size);
    hash_table->values = hash_table->keys;
    hash_table->hashes = sys_new0(SysUInt, hash_table->size);
  } else {
    hash_table->keys = NULL;
    hash_table->values = NULL;
    hash_table->hashes = NULL;
  }

  for (i = 0; i < old_size; i++) {
    if (HASH_IS_REAL(old_hashes[i])) {
      key = old_keys[i];
      value = old_values[i];

      old_hashes[i] = UNUSED_HASH_VALUE;
      old_keys[i] = NULL;
      old_values[i] = NULL;

      if (hash_table->key_destroy_func != NULL) {
        hash_table->key_destroy_func(key);
      }

      if (hash_table->value_destroy_func != NULL) {
        hash_table->value_destroy_func(value);
      }
    }
  }

  /* Destroy old storage space. */
  if (old_keys != old_values)
    sys_free(old_values);

  sys_free(old_keys);
  sys_free(old_hashes);
}

static void sys_hash_table_resize(SysHashTable *hash_table) {
  SysPointer *new_keys;
  SysPointer *new_values;
  SysUInt *new_hashes;
  SysInt old_size;
  SysInt i;

  old_size = hash_table->size;
  sys_hash_table_set_shift_from_size(hash_table, hash_table->nnodes * 2);

  new_keys = sys_new0(SysPointer, hash_table->size);
  if (hash_table->keys == hash_table->values)
    new_values = new_keys;
  else
    new_values = sys_new0(SysPointer, hash_table->size);
  new_hashes = sys_new0(SysUInt, hash_table->size);

  for (i = 0; i < old_size; i++) {
    SysUInt node_hash = hash_table->hashes[i];
    SysUInt hash_val;
    SysUInt step = 0;

    if (!HASH_IS_REAL(node_hash))
      continue;

    hash_val = node_hash % hash_table->mod;

    while (!HASH_IS_UNUSED(new_hashes[hash_val])) {
      step++;
      hash_val += step;
      hash_val &= hash_table->mask;
    }

    new_hashes[hash_val] = hash_table->hashes[i];
    new_keys[hash_val] = hash_table->keys[i];
    new_values[hash_val] = hash_table->values[i];
  }

  if (hash_table->keys != hash_table->values)
    sys_free(hash_table->values);

  sys_free(hash_table->keys);
  sys_free(hash_table->hashes);

  hash_table->keys = new_keys;
  hash_table->values = new_values;
  hash_table->hashes = new_hashes;

  hash_table->noccupied = hash_table->nnodes;
}

static inline void sys_hash_table_maybe_resize(SysHashTable *hash_table) {
  SysInt noccupied = hash_table->noccupied;
  SysInt size = hash_table->size;

  if ((size > hash_table->nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT) ||
      (size <= noccupied + (noccupied / 16)))
    sys_hash_table_resize(hash_table);
}

SysHashTable *sys_hash_table_new(SysHashFunc hash_func,
    SysEqualFunc key_equal_func) {
  return sys_hash_table_new_full(hash_func, key_equal_func, NULL, NULL);
}

SysHashTable *sys_hash_table_new_full(SysHashFunc hash_func,
    SysEqualFunc key_equal_func,
    SysDestroyFunc key_destroy_func,
    SysDestroyFunc value_destroy_func) {
  SysHashTable *hash_table;

  hash_table = sys_block_new(SysHashTable, 1);
  sys_hash_table_set_shift(hash_table, HASH_TABLE_MIN_SHIFT);
  hash_table->nnodes = 0;
  hash_table->noccupied = 0;
  hash_table->hash_func = hash_func ? hash_func : sys_direct_hash;
  hash_table->key_equal_func = key_equal_func;

  hash_table->key_destroy_func = key_destroy_func;
  hash_table->value_destroy_func = value_destroy_func;
  hash_table->keys = sys_new0(SysPointer, hash_table->size);
  hash_table->values = hash_table->keys;
  hash_table->hashes = sys_new0(SysUInt, hash_table->size);

  return hash_table;
}

void sys_hash_table_iter_init(SysHashTableIter *iter,
                              SysHashTable *hash_table) {
  RealIter *ri = (RealIter *)iter;

  sys_return_if_fail(iter != NULL);
  sys_return_if_fail(hash_table != NULL);

  ri->hash_table = hash_table;
  ri->position = -1;
}

SysBool sys_hash_table_iter_next(SysHashTableIter *iter, SysPointer *key,
                              SysPointer *value) {
  RealIter *ri = (RealIter *)iter;
  SysInt position;

  sys_return_val_if_fail(iter != NULL, false);
  sys_return_val_if_fail(ri->position < ri->hash_table->size, false);

  position = ri->position;

  do {
    position++;
    if (position >= ri->hash_table->size) {
      ri->position = position;
      return false;
    }
  } while (!HASH_IS_REAL(ri->hash_table->hashes[position]));

  if (key != NULL)
    *key = ri->hash_table->keys[position];
  if (value != NULL)
    *value = ri->hash_table->values[position];

  ri->position = position;
  return true;
}

SysHashTable *sys_hash_table_iter_get_hash_table(SysHashTableIter *iter) {
  sys_return_val_if_fail(iter != NULL, NULL);

  return ((RealIter *)iter)->hash_table;
}

static void iter_remove_or_steal(RealIter *ri, SysBool notify) {
  sys_return_if_fail(ri != NULL);
  sys_return_if_fail(ri->position >= 0);
  sys_return_if_fail(ri->position < ri->hash_table->size);

  sys_hash_table_remove_node(ri->hash_table, ri->position, notify);
}

void sys_hash_table_iter_remove(SysHashTableIter *iter) {
  iter_remove_or_steal((RealIter *)iter, true);
}

static SysBool sys_hash_table_insert_node(SysHashTable *hash_table,
                                       SysUInt node_index, SysUInt key_hash,
                                       SysPointer new_key, SysPointer new_value,
                                       SysBool keep_new_key, SysBool reusinsys_key) {
  SysBool already_exists;
  SysUInt old_hash;
  SysPointer key_to_free = NULL;
  SysPointer value_to_free = NULL;

  old_hash = hash_table->hashes[node_index];
  already_exists = HASH_IS_REAL(old_hash);

  /* Proceed in three steps.  First, deal with the key because it is the
   * most complicated.  Then consider if we need to split the table in
   * two (because writing the value will result in the set invariant
   * becoming broken).  Then deal with the value.
   *
   * There are three cases for the key:
   *
   *  - entry already exists in table, reusing key:
   *    free the just-passed-in new_key and use the existing value
   *
   *  - entry already exists in table, not reusing key:
   *    free the entry in the table, use the new key
   *
   *  - entry not already in table:
   *    use the new key, free nothing
   *
   * We update the hash at the same time...
   */
  if (already_exists) {
    /* Note: we must record the old value before writing the new key
     * because we might change the value in the event that the two
     * arrays are shared.
     */
    value_to_free = hash_table->values[node_index];

    if (keep_new_key) {
      key_to_free = hash_table->keys[node_index];
      hash_table->keys[node_index] = new_key;
    } else
      key_to_free = new_key;
  } else {
    hash_table->hashes[node_index] = key_hash;
    hash_table->keys[node_index] = new_key;
  }

  /* Step two: check if the value that we are about to write to the
   * table is the same as the key in the same position.  If it's not,
   * split the table.
   */
  if (hash_table->keys == hash_table->values &&
      hash_table->keys[node_index] != new_value) {
    hash_table->values = sys_memdup(hash_table->keys, sizeof(SysPointer) * hash_table->size);
  }

  /* Step 3: Actually do the write */
  hash_table->values[node_index] = new_value;

  /* Now, the bookkeeping... */
  if (!already_exists) {
    hash_table->nnodes++;

    if (HASH_IS_UNUSED(old_hash)) {
      /* We replaced an empty node, and not a tombstone */
      hash_table->noccupied++;
      sys_hash_table_maybe_resize(hash_table);
    }
  }

  if (already_exists) {
    if (hash_table->key_destroy_func && !reusinsys_key) {
      hash_table->key_destroy_func(key_to_free);
    }
    if (hash_table->value_destroy_func) {
      hash_table->value_destroy_func(value_to_free);
    }
  }

  return !already_exists;
}

void sys_hash_table_iter_replace(SysHashTableIter *iter, SysPointer value) {
  RealIter *ri;
  SysUInt node_hash;
  SysPointer key;

  ri = (RealIter *)iter;

  sys_return_if_fail(ri != NULL);
  sys_return_if_fail(ri->position >= 0);
  sys_return_if_fail(ri->position < ri->hash_table->size);

  node_hash = ri->hash_table->hashes[ri->position];
  key = ri->hash_table->keys[ri->position];

  sys_hash_table_insert_node(ri->hash_table, ri->position, node_hash, key,
                             value, true, true);
}

void sys_hash_table_iter_steal(SysHashTableIter *iter) {
  iter_remove_or_steal((RealIter *)iter, false);
}

SysHashTable *sys_hash_table_ref(SysHashTable *hash_table) {
  sys_return_val_if_fail(hash_table != NULL, NULL);

  sys_block_ref_inc(hash_table);

  return hash_table;
}

void sys_hash_table_unref(SysHashTable *hash_table) {
  sys_return_if_fail(hash_table != NULL);

  if (sys_block_ref_dec(hash_table)) {
    sys_hash_table_remove_all_nodes(hash_table, true, true);
    if (hash_table->keys != hash_table->values)
      sys_free(hash_table->values);

    if (hash_table->keys != NULL) {
      sys_free(hash_table->keys);
      sys_free(hash_table->hashes);
    }

    sys_block_free(hash_table);
  }
}

void sys_hash_table_free(SysHashTable *hash_table) {
  sys_return_if_fail(hash_table != NULL);

  sys_hash_table_remove_all(hash_table);
  sys_hash_table_unref(hash_table);
}

SysPointer sys_hash_table_lookup(SysHashTable *hash_table,
    const SysPointer key) {
  SysUInt node_index;
  SysUInt node_hash;

  sys_return_val_if_fail(hash_table != NULL, NULL);

  node_index = sys_hash_table_lookup_node(hash_table, key, &node_hash);

  return HASH_IS_REAL(hash_table->hashes[node_index])
    ? hash_table->values[node_index]
    : NULL;
}

SysBool sys_hash_table_lookup_extended(SysHashTable *hash_table,
    const SysPointer lookup_key,
    SysPointer *orisys_key, SysPointer *value) {
  SysUInt node_index;
  SysUInt node_hash;

  sys_return_val_if_fail(hash_table != NULL, false);

  node_index = sys_hash_table_lookup_node(hash_table, lookup_key, &node_hash);

  if (!HASH_IS_REAL(hash_table->hashes[node_index]))
    return false;

  if (orisys_key)
    *orisys_key = hash_table->keys[node_index];

  if (value)
    *value = hash_table->values[node_index];

  return true;
}

static SysBool sys_hash_table_insert_internal(SysHashTable *hash_table,
    SysPointer key, SysPointer value,
    SysBool keep_new_key) {
  SysUInt key_hash;
  SysUInt node_index;

  sys_return_val_if_fail(hash_table != NULL, false);

  node_index = sys_hash_table_lookup_node(hash_table, key, &key_hash);

  return sys_hash_table_insert_node(hash_table, node_index, key_hash, key,
      value, keep_new_key, false);
}

SysBool sys_hash_table_insert(SysHashTable *hash_table, SysPointer key,
    SysPointer value) {
  return sys_hash_table_insert_internal(hash_table, key, value, false);
}

SysBool sys_hash_table_replace(SysHashTable *hash_table, SysPointer key,
    SysPointer value) {
  return sys_hash_table_insert_internal(hash_table, key, value, true);
}

SysBool sys_hash_table_add(SysHashTable *hash_table, SysPointer key) {
  return sys_hash_table_insert_internal(hash_table, key, key, true);
}

SysBool sys_hash_table_contains(SysHashTable *hash_table, const SysPointer key) {
  SysUInt node_index;
  SysUInt node_hash;

  sys_return_val_if_fail(hash_table != NULL, false);

  node_index = sys_hash_table_lookup_node(hash_table, key, &node_hash);

  return HASH_IS_REAL(hash_table->hashes[node_index]);
}

static SysBool sys_hash_table_remove_internal(SysHashTable *hash_table,
    const SysPointer key, SysBool notify) {
  SysUInt node_index;
  SysUInt node_hash;

  sys_return_val_if_fail(hash_table != NULL, false);

  node_index = sys_hash_table_lookup_node(hash_table, key, &node_hash);

  if (!HASH_IS_REAL(hash_table->hashes[node_index]))
    return false;

  sys_hash_table_remove_node(hash_table, node_index, notify);
  sys_hash_table_maybe_resize(hash_table);

  return true;
}

SysBool sys_hash_table_remove(SysHashTable *hash_table, const SysPointer key) {
  return sys_hash_table_remove_internal(hash_table, key, true);
}

SysBool sys_hash_table_steal(SysHashTable *hash_table, const SysPointer key) {
  return sys_hash_table_remove_internal(hash_table, key, false);
}

void sys_hash_table_remove_all(SysHashTable *hash_table) {
  sys_return_if_fail(hash_table != NULL);

  sys_hash_table_remove_all_nodes(hash_table, true, false);
  sys_hash_table_maybe_resize(hash_table);
}

void sys_hash_table_steal_all(SysHashTable *hash_table) {
  sys_return_if_fail(hash_table != NULL);

  sys_hash_table_remove_all_nodes(hash_table, false, false);
  sys_hash_table_maybe_resize(hash_table);
}

static SysUInt sys_hash_table_foreach_remove_or_steal(SysHashTable *hash_table,
    SysHRFunc func,
    SysPointer user_data,
    SysBool notify) {
  SysUInt deleted = 0;
  SysInt i;

  for (i = 0; i < hash_table->size; i++) {
    SysUInt node_hash = hash_table->hashes[i];
    SysPointer node_key = hash_table->keys[i];
    SysPointer node_value = hash_table->values[i];

    if (HASH_IS_REAL(node_hash) && (*func)(node_key, node_value, user_data)) {
      sys_hash_table_remove_node(hash_table, i, notify);
      deleted++;
    }
  }

  sys_hash_table_maybe_resize(hash_table);

  return deleted;
}

SysUInt sys_hash_table_foreach_remove(SysHashTable *hash_table, SysHRFunc func,
    SysPointer user_data) {
  sys_return_val_if_fail(hash_table != NULL, 0);
  sys_return_val_if_fail(func != NULL, 0);

  return sys_hash_table_foreach_remove_or_steal(hash_table, func, user_data,
      true);
}

SysUInt sys_hash_table_foreach_steal(SysHashTable *hash_table, SysHRFunc func,
    SysPointer user_data) {
  sys_return_val_if_fail(hash_table != NULL, 0);
  sys_return_val_if_fail(func != NULL, 0);

  return sys_hash_table_foreach_remove_or_steal(hash_table, func, user_data,
      false);
}

void sys_hash_table_foreach(SysHashTable *hash_table, SysHFunc func,
    SysPointer user_data) {
  SysInt i;

  sys_return_if_fail(hash_table != NULL);
  sys_return_if_fail(func != NULL);

  for (i = 0; i < hash_table->size; i++) {
    SysUInt node_hash = hash_table->hashes[i];
    SysPointer node_key = hash_table->keys[i];
    SysPointer node_value = hash_table->values[i];

    if (HASH_IS_REAL(node_hash))
      (*func)(node_key, node_value, user_data);
  }
}

SysPointer sys_hash_table_find(SysHashTable *hash_table, SysHRFunc predicate,
    SysPointer user_data) {
  SysInt i;
  SysBool match;

  sys_return_val_if_fail(hash_table != NULL, NULL);
  sys_return_val_if_fail(predicate != NULL, NULL);

  match = false;

  for (i = 0; i < hash_table->size; i++) {
    SysUInt node_hash = hash_table->hashes[i];
    SysPointer node_key = hash_table->keys[i];
    SysPointer node_value = hash_table->values[i];

    if (HASH_IS_REAL(node_hash))
      match = predicate(node_key, node_value, user_data);

    if (match)
      return node_value;
  }

  return NULL;
}

SysUInt sys_hash_table_size(SysHashTable *hash_table) {
  sys_return_val_if_fail(hash_table != NULL, 0);

  return hash_table->nnodes;
}

SysPtrArray *sys_hash_table_get_keys(SysHashTable *hash_table) {
  SysInt i;
  SysPtrArray *retval;

  sys_return_val_if_fail(hash_table != NULL, NULL);

  retval = sys_ptr_array_new_with_free_func(hash_table->key_destroy_func);
  for (i = 0; i < hash_table->size; i++) {
    if (HASH_IS_REAL(hash_table->hashes[i]))
      sys_ptr_array_add(retval, hash_table->keys[i]);
  }

  return retval;
}

SysPointer *sys_hash_table_get_keys_as_array(SysHashTable *hash_table,
    SysUInt *length) {
  SysPointer *result;
  SysUInt i, j = 0;

  result = sys_new(SysPointer, hash_table->nnodes + 1);
  for (i = 0; i < (SysUInt)hash_table->size; i++) {
    if (HASH_IS_REAL(hash_table->hashes[i]))
      result[j++] = hash_table->keys[i];
  }
  sys_return_val_if_fail(j == (SysUInt)hash_table->nnodes, NULL);

  result[j] = NULL;

  if (length)
    *length = j;

  return result;
}

SysPtrArray *sys_hash_table_get_values(SysHashTable *hash_table) {
  SysInt i;
  SysPtrArray *retval;

  sys_return_val_if_fail(hash_table != NULL, NULL);

  retval = sys_ptr_array_new_with_free_func(hash_table->value_destroy_func);
  for (i = 0; i < hash_table->size; i++) {
    if (HASH_IS_REAL(hash_table->hashes[i]))
      sys_ptr_array_add(retval, hash_table->values[i]);
  }

  return retval;
}

SysUInt sys_str_hash(const SysPointer v) {
  const SysInt8 *p;
  SysUInt32 h = 5381;

  for (p = v; *p != '\0'; p++)
    h = (h << 5) + h + *p;

  return h;
}

SysUInt sys_direct_hash(const SysPointer v) {
  return POINTER_TO_UINT(v);
}

SysBool sys_direct_equal(const SysPointer v1, const SysPointer v2) {
  return v1 == v2;
}

SysBool sys_int_equal(const SysPointer v1, const SysPointer v2) {
  return *((const SysInt *)v1) == *((const SysInt *)v2);
}

SysUInt sys_int_hash(const SysPointer v) {
  return *(const SysInt *)v;
}

SysBool sys_int64_equal(const SysPointer v1, const SysPointer v2) {
  return *((const SysInt64 *)v1) == *((const SysInt64 *)v2);
}

SysUInt sys_int64_hash(const SysPointer v) {
  return (SysUInt) * (const SysInt64 *)v;
}

SysBool sys_double_equal(const SysPointer v1, const SysPointer v2) {
  return *((const SysDouble *)v1) == *((const SysDouble *)v2);
}

SysUInt sys_double_hash(const SysPointer v) {
  return (SysUInt) * (const SysDouble *)v;
}
