#ifndef __SYS_HASH_TABLE_H__
#define __SYS_HASH_TABLE_H__

#include <System/DataTypes/SysArray.h>

/**
 * this code from glib hashtable
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

SYS_BEGIN_DECLS

typedef struct _SysHashTable SysHashTable;
typedef struct _SysHashTableIter SysHashTableIter;

typedef SysUInt (*SysHashFunc)(const SysPointer key);
typedef SysBool (*SysHRFunc)(SysPointer key, SysPointer value, SysPointer user_data);
typedef void (*SysHFunc) (SysPointer key, SysPointer value, SysPointer user_data);

struct _SysHashTableIter {
  /*< private >*/
  SysPointer      dummy1;
  SysPointer      dummy2;
  SysPointer      dummy3;
  int             dummy4;
  bool            dummy5;
  SysPointer      dummy6;
};

SYS_API SysHashTable *sys_hash_table_new(SysHashFunc hash_func, SysEqualFunc key_equal_func);
SYS_API SysHashTable *sys_hash_table_new_full(SysHashFunc hash_func,
                                  SysEqualFunc key_equal_func,
                                  SysDestroyFunc key_destroy_func,
                                  SysDestroyFunc value_destroy_func);
SYS_API void sys_hash_table_free(SysHashTable *hash_table);
SYS_API bool sys_hash_table_insert(SysHashTable *hash_table, SysPointer key,
                             SysPointer value);
SYS_API bool sys_hash_table_replace(SysHashTable *hash_table, SysPointer key,
                              SysPointer value);
SYS_API bool sys_hash_table_add(SysHashTable *hash_table, SysPointer key);
SYS_API bool sys_hash_table_remove(SysHashTable *hash_table, const SysPointer key);
SYS_API void sys_hash_table_remove_all(SysHashTable *hash_table);
SYS_API bool sys_hash_table_steal(SysHashTable *hash_table, const SysPointer key);
SYS_API void sys_hash_table_steal_all(SysHashTable *hash_table);
SYS_API SysPointer sys_hash_table_lookup(SysHashTable *hash_table, const SysPointer key);
SYS_API bool sys_hash_table_contains(SysHashTable *hash_table, const SysPointer key);
SYS_API bool sys_hash_table_lookup_extended(SysHashTable *hash_table,
                                      const SysPointer lookup_key,
                                      SysPointer *orisys_key, SysPointer *value);
SYS_API void sys_hash_table_foreach(SysHashTable *hash_table, SysHFunc func,
                          SysPointer user_data);
SYS_API SysPointer sys_hash_table_find(SysHashTable *hash_table, SysHRFunc predicate,
                           SysPointer user_data);
SYS_API SysUInt sys_hash_table_foreach_remove(SysHashTable *hash_table, SysHRFunc func,
                                  SysPointer user_data);
SYS_API SysUInt sys_hash_table_foreach_steal(SysHashTable *hash_table, SysHRFunc func,
                                 SysPointer user_data);
SYS_API SysUInt sys_hash_table_size(SysHashTable *hash_table);
SYS_API SysPtrArray *sys_hash_table_get_keys(SysHashTable *hash_table);
SYS_API SysPtrArray *sys_hash_table_get_values(SysHashTable *hash_table);
SYS_API SysPointer *sys_hash_table_get_keys_as_array(SysHashTable *hash_table, SysUInt *length);

SYS_API void sys_hash_table_iter_init(SysHashTableIter *iter, SysHashTable *hash_table);
SYS_API bool sys_hash_table_iter_next(SysHashTableIter *iter, SysPointer *key,
                                SysPointer *value);
SYS_API SysHashTable *sys_hash_table_iter_get_hash_table(SysHashTableIter *iter);
SYS_API void sys_hash_table_iter_remove(SysHashTableIter *iter);

SYS_API void sys_hash_table_iter_replace(SysHashTableIter *iter, SysPointer value);
SYS_API void sys_hash_table_iter_steal(SysHashTableIter *iter);

SYS_API SysHashTable *sys_hash_table_ref(SysHashTable *hash_table);
SYS_API void sys_hash_table_unref(SysHashTable *hash_table);

/* Hash Functions
 */
SYS_API SysUInt sys_str_hash(const SysPointer v);

SYS_API bool sys_int_equal(const SysPointer v1, const SysPointer v2);
SYS_API SysUInt sys_int_hash(const SysPointer v);

SYS_API bool sys_int64_equal(const SysPointer v1, const SysPointer v2);
SYS_API SysUInt sys_int64_hash(const SysPointer v);

SYS_API bool sys_double_equal(const SysPointer v1, const SysPointer v2);
SYS_API SysUInt sys_double_hash(const SysPointer v);

SYS_API SysUInt sys_direct_hash(const SysPointer v);
SYS_API bool sys_direct_equal(const SysPointer v1, const SysPointer v2);

SYS_END_DECLS

#endif
