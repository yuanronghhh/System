#ifndef __SYS_ARRAY_H__
#define __SYS_ARRAY_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

struct _SysArray {
  void** pdata;
  SysUInt len;
};

struct _SysPtrArray {
  void** pdata;
  SysUInt    len;
};

struct _SysByteArray {
  void** pdata;
  SysUInt len;
};


/* Resizable arrays. remove fills any cleared spot and shortens the
 * array, while preserving the order. remove_fast will distort the
 * order by moving the last element to the position of the removed.
 */

#define sys_array_append_val(a,v)	  sys_array_append_vals (a, &(v), 1)
#define sys_array_prepend_val(a,v)  sys_array_prepend_vals (a, &(v), 1)
#define sys_array_insert_val(a,i,v) sys_array_insert_vals (a, i, &(v), 1)
#define sys_array_index(a,t,i)      (((t*) (void *) (a)->pdata) [(i)])

SYS_API SysArray* sys_array_new(SysBool zero_terminated, SysBool clear_, SysUInt element_size);
SYS_API SysPointer sys_array_steal(SysArray *array, SysSize *len);
SYS_API SysArray* sys_array_sized_new(SysBool zero_terminated, SysBool clear_, SysUInt element_size, SysUInt reserved_size);
SYS_API SysArray* sys_array_copy(SysArray *array);
SYS_API SysChar* sys_array_free(SysArray *array, SysBool free_segment);
SYS_API SysArray *sys_array_ref(SysArray *array);
SYS_API void sys_array_unref(SysArray *array);
SYS_API SysUInt sys_array_get_element_size(SysArray *array);
SYS_API SysArray* sys_array_append_vals(SysArray *array, const SysPointer data, SysUInt len);
SYS_API SysArray* sys_array_prepend_vals(SysArray *array, const SysPointer data, SysUInt len);
SYS_API SysArray* sys_array_insert_vals(SysArray *array, SysUInt index_, const SysPointer data, SysUInt len);
SYS_API SysArray* sys_array_set_size(SysArray *array, SysUInt length);
SYS_API SysArray* sys_array_remove_index(SysArray *array, SysUInt index_);
SYS_API SysArray* sys_array_remove_index_fast(SysArray *array, SysUInt index_);
SYS_API SysArray* sys_array_remove_range(SysArray *array, SysUInt index_, SysUInt length);
SYS_API SysBool sys_array_binary_search(SysArray *array, const SysPointer target, SysCompareFunc compare_func, SysUInt *out_match_index);
SYS_API void sys_array_set_clear_func(SysArray *array, SysDestroyFunc clear_func);

/* Resizable pointer array.  This interface is much less complicated
 * than the above.  Add appends a pointer.  Remove fills any cleared
 * spot and shortens the array. remove_fast will again distort order.
 */
#define    sys_ptr_array_index(array,index_) ((array)->pdata)[index_]
SYS_API SysPtrArray* sys_ptr_array_new(void);
SYS_API SysPtrArray* sys_ptr_array_new_with_free_func(SysDestroyFunc element_free_func);
SYS_API SysPointer* sys_ptr_array_steal(SysPtrArray *array, SysSize *len);
SYS_API SysPtrArray *sys_ptr_array_copy(SysPtrArray *array, SysCopyFunc func, SysPointer user_data);
SYS_API SysPtrArray* sys_ptr_array_sized_new(SysUInt reserved_size);
SYS_API SysPtrArray* sys_ptr_array_new_full(SysUInt reserved_size, SysDestroyFunc element_free_func);
SYS_API SysPointer* sys_ptr_array_free(SysPtrArray *array, SysBool free_seg);
SYS_API SysPtrArray* sys_ptr_array_ref(SysPtrArray *array);
SYS_API void sys_ptr_array_unref(SysPtrArray *array);
SYS_API void sys_ptr_array_set_free_func(SysPtrArray *array, SysDestroyFunc element_free_func);
SYS_API void sys_ptr_array_set_size(SysPtrArray *array, SysInt length);
SYS_API SysPointer sys_ptr_array_remove_index(SysPtrArray *array, SysUInt index_);
SYS_API SysPointer sys_ptr_array_remove_index_fast(SysPtrArray *array, SysUInt index_);
SYS_API SysPointer sys_ptr_array_steal_index(SysPtrArray *array, SysUInt index_);
SYS_API SysPointer sys_ptr_array_steal_index_fast(SysPtrArray *array, SysUInt index_);
SYS_API SysBool sys_ptr_array_remove(SysPtrArray *array, SysPointer data);
SYS_API SysBool sys_ptr_array_remove_fast(SysPtrArray *array, SysPointer data);
SYS_API SysPtrArray *sys_ptr_array_remove_range(SysPtrArray *array, SysUInt index_, SysUInt length);
SYS_API void sys_ptr_array_add(SysPtrArray *array, SysPointer data);
SYS_API void sys_ptr_array_extend(SysPtrArray *array_to_extend, SysPtrArray *array, SysCopyFunc func, SysPointer user_data);
SYS_API void sys_ptr_array_extend_and_steal(SysPtrArray *array_to_extend, SysPtrArray *array);
SYS_API void sys_ptr_array_insert(SysPtrArray *array, SysInt index_, SysPointer data);
SYS_API void sys_ptr_array_foreach(SysPtrArray *array, SysFunc func, SysPointer user_data);
SYS_API SysBool sys_ptr_array_find(SysPtrArray *haystack, const SysPointer needle, SysUInt *index_);
SYS_API SysBool sys_ptr_array_find_with_equal_func(SysPtrArray *haystack, const SysPointer needle, SysEqualFunc equal_func, SysUInt *index_);

/* Byte arrays, an array of SysUInt8.  Implemented as a SysArray,
 * but type-safe.
 */

SYS_API SysByteArray* sys_byte_array_new(void);
SYS_API SysByteArray* sys_byte_array_new_take(SysUInt8           *data, SysSize len);
SYS_API SysUInt8*     sys_byte_array_steal(SysByteArray       *array, SysSize *len);
SYS_API SysByteArray* sys_byte_array_sized_new(SysUInt             reserved_size);
SYS_API SysUInt8*     sys_byte_array_free(SysByteArray       *array, SysBool          free_segment);
SYS_API SysByteArray *sys_byte_array_ref(SysByteArray       *array);
SYS_API void        sys_byte_array_unref(SysByteArray       *array);
SYS_API SysByteArray* sys_byte_array_append(SysByteArray *array, const SysUInt8 *data, SysUInt len);
SYS_API SysByteArray* sys_byte_array_prepend(SysByteArray *array, const SysUInt8 *data, SysUInt len);
SYS_API SysByteArray* sys_byte_array_set_size(SysByteArray *array, SysUInt length);
SYS_API SysByteArray* sys_byte_array_remove_index(SysByteArray *array, SysUInt index_);
SYS_API SysByteArray* sys_byte_array_remove_index_fast(SysByteArray *array, SysUInt index_);
SYS_API SysByteArray* sys_byte_array_remove_range(SysByteArray *array, SysUInt index_, SysUInt length);
SYS_API void sys_byte_array_sort(SysByteArray *array, SysCompareFunc compare_func);
SYS_API void sys_byte_array_sort_with_data(SysByteArray *array, SysCompareDataFunc compare_func, SysPointer user_data);

SYS_END_DECLS

#endif
