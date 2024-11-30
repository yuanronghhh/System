#include <System/DataTypes/SysArray.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/Utils/SysStr.h>
#include <System/Type/SysBlock.h>

/**
 * this code from glib array
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

#define MIN_ARRAY_SIZE  16

typedef struct _SysRealArray  SysRealArray;

struct _SysRealArray
{
  SysUInt8 *data;
  SysUInt   len;
  SysUInt   alloc;
  SysUInt   elt_size;
  SysUInt   zero_terminated : 1;
  SysUInt   clear : 1;
  SysRef ref_count;
  SysDestroyFunc clear_func;
};

#define sys_array_elt_len(array,i) ((array)->elt_size * (i))
#define sys_array_elt_pos(array,i) ((array)->data + sys_array_elt_len((array),(i)))
#define sys_array_elt_zero(array, pos, len)                               \
  (memset (sys_array_elt_pos ((array), pos), 0,  sys_array_elt_len ((array), len)))
#define sys_array_zero_terminate(array) do {                     \
  if ((array)->zero_terminated)                                         \
    sys_array_elt_zero ((array), (array)->len, 1);                        \
}while(0)

static void  sys_array_maybe_expand(SysRealArray *array,
    SysUInt       len);

SysArray* sys_array_new(SysBool zero_terminated,
    SysBool clear,
    SysUInt    elt_size) {
    sys_return_val_if_fail(elt_size > 0, NULL);

    return sys_array_sized_new(zero_terminated, clear, elt_size, 0);
}

SysPointer sys_array_steal(SysArray *array,
    SysSize *len) {
    SysRealArray *rarray;
    SysPointer segment;

    sys_return_val_if_fail(array != NULL, NULL);

    rarray = (SysRealArray *)array;
    segment = (SysPointer)rarray->data;

    if (len != NULL)
        *len = rarray->len;

    rarray->data = NULL;
    rarray->len = 0;
    rarray->alloc = 0;
    return segment;
}

SysArray* sys_array_sized_new(SysBool zero_terminated,
    SysBool clear,
    SysUInt    elt_size,
    SysUInt    reserved_size) {
    SysRealArray *array;

    sys_return_val_if_fail(elt_size > 0, NULL);

    array = sys_slice_new(SysRealArray);

    array->data = NULL;
    array->len = 0;
    array->alloc = 0;
    array->zero_terminated = (zero_terminated ? 1 : 0);
    array->clear = (clear ? 1 : 0);
    array->elt_size = elt_size;
    array->clear_func = NULL;

    sys_block_ref_count_init(array);

    if (array->zero_terminated || reserved_size != 0)
    {
        sys_array_maybe_expand(array, reserved_size);
        sys_array_zero_terminate(array);
    }

    return (SysArray*)array;
}

void sys_array_set_clear_func(SysArray         *array,
    SysDestroyFunc  clear_func) {
    SysRealArray *rarray = (SysRealArray *)array;

    sys_return_if_fail(array != NULL);

    rarray->clear_func = clear_func;
}

SysArray * sys_array_ref(SysArray *array) {
    SysRealArray *rarray = (SysRealArray*)array;
    sys_return_val_if_fail(array, NULL);

    sys_block_ref_count_inc(rarray);

    return array;
}

typedef enum
{
    FREE_SEGMENT = 1 << 0,
    PRESERVE_WRAPPER = 1 << 1
} ArrayFreeFlags;

static SysChar *array_free(SysRealArray *, ArrayFreeFlags);

void sys_array_unref(SysArray *array) {
    SysRealArray *rarray = (SysRealArray*)array;
    sys_return_if_fail(array);

    if (sys_block_ref_count_dec(rarray))
        array_free(rarray, FREE_SEGMENT);
}

SysUInt sys_array_get_element_size(SysArray *array) {
    SysRealArray *rarray = (SysRealArray*)array;

    sys_return_val_if_fail(array, 0);

    return rarray->elt_size;
}

SysChar* sys_array_free(SysArray   *farray,
    SysBool  free_segment) {
    SysRealArray *array = (SysRealArray*)farray;
    ArrayFreeFlags flags;

    sys_return_val_if_fail(array, NULL);

    flags = (free_segment ? FREE_SEGMENT : 0);

        if (!sys_block_ref_count_dec(array))
        flags |= PRESERVE_WRAPPER;

    return array_free(array, flags);
}

static SysChar * array_free(SysRealArray     *array,
    ArrayFreeFlags  flags) {
    SysChar *segment;

    if (flags & FREE_SEGMENT)
    {
        if (array->clear_func != NULL)
        {
            SysUInt i;

            for (i = 0; i < array->len; i++)
                array->clear_func(sys_array_elt_pos(array, i));
        }

        sys_free(array->data);
        segment = NULL;
    }
    else
        segment = (SysChar*)array->data;

    if (flags & PRESERVE_WRAPPER)
    {
        array->data = NULL;
        array->len = 0;
        array->alloc = 0;
    }
    else
    {
        sys_slice_free1(sizeof(SysRealArray), array);
    }

    return segment;
}

 SysArray* sys_array_append_vals(SysArray       *farray,
    const SysPointer data,
    SysUInt         len) {
    SysRealArray *array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);

    if (len == 0)
        return farray;

    sys_array_maybe_expand(array, len);

    memcpy(sys_array_elt_pos(array, array->len), data,
        sys_array_elt_len(array, len));

    array->len += len;

    sys_array_zero_terminate(array);

    return farray;
}

 SysArray* sys_array_prepend_vals(SysArray        *farray,
    const SysPointer  data,
    SysUInt          len) {
    SysRealArray *array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);

    if (len == 0)
        return farray;

    sys_array_maybe_expand(array, len);

    memmove(sys_array_elt_pos(array, len), sys_array_elt_pos(array, 0),
        sys_array_elt_len(array, array->len));

    memcpy(sys_array_elt_pos(array, 0), data, sys_array_elt_len(array, len));

    array->len += len;

    sys_array_zero_terminate(array);

    return farray;
}

 SysArray* sys_array_insert_vals(SysArray        *farray,
    SysUInt          index_,
    const SysPointer  data,
    SysUInt          len) {
    SysRealArray *array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);

    if (len == 0)
        return farray;

    /* Is the index off the end of the array, and hence do we need to over-allocate
     * and clear some elements? */
    if (index_ >= array->len)
    {
        sys_array_maybe_expand(array, index_ - array->len + len);
        return sys_array_append_vals(sys_array_set_size(farray, index_), data, len);
    }

    sys_array_maybe_expand(array, len);

    memmove(sys_array_elt_pos(array, len + index_),
        sys_array_elt_pos(array, index_),
        sys_array_elt_len(array, array->len - index_));

    memcpy(sys_array_elt_pos(array, index_), data, sys_array_elt_len(array, len));

    array->len += len;

    sys_array_zero_terminate(array);

    return farray;
}

SysArray* sys_array_set_size(SysArray *farray,
    SysUInt   length) {
    SysRealArray *array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);

    if (length > array->len)
    {
        sys_array_maybe_expand(array, length - array->len);

        if (array->clear)
            sys_array_elt_zero(array, array->len, length - array->len);
    }
    else if (length < array->len)
        sys_array_remove_range(farray, length, array->len - length);

    array->len = length;

    sys_array_zero_terminate(array);

    return farray;
}

SysArray* sys_array_remove_index(SysArray *farray,
    SysUInt   index_) {
    SysRealArray* array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);

    sys_return_val_if_fail(index_ < array->len, NULL);

    if (array->clear_func != NULL)
        array->clear_func(sys_array_elt_pos(array, index_));

    if (index_ != array->len - 1)
        memmove(sys_array_elt_pos(array, index_),
            sys_array_elt_pos(array, index_ + 1),
            sys_array_elt_len(array, array->len - index_ - 1));

    array->len -= 1;

        sys_array_zero_terminate(array);

    return farray;
}

SysArray* sys_array_remove_index_fast(SysArray *farray,
    SysUInt   index_) {
    SysRealArray* array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);

    sys_return_val_if_fail(index_ < array->len, NULL);

    if (array->clear_func != NULL)
        array->clear_func(sys_array_elt_pos(array, index_));

    if (index_ != array->len - 1)
        memcpy(sys_array_elt_pos(array, index_),
            sys_array_elt_pos(array, array->len - 1),
            sys_array_elt_len(array, 1));

    array->len -= 1;
    sys_array_zero_terminate(array);

    return farray;
}

SysArray* sys_array_remove_range(SysArray *farray,
    SysUInt   index_,
    SysUInt   length) {
    SysRealArray *array = (SysRealArray*)farray;

    sys_return_val_if_fail(array, NULL);
    sys_return_val_if_fail(index_ <= array->len, NULL);
    sys_return_val_if_fail(index_ + length <= array->len, NULL);

    if (array->clear_func != NULL)
    {
        SysUInt i;

        for (i = 0; i < length; i++)
            array->clear_func(sys_array_elt_pos(array, index_ + i));
    }

    if (index_ + length != array->len)
        memmove(sys_array_elt_pos(array, index_),
            sys_array_elt_pos(array, index_ + length),
            (array->len - (index_ + length)) * array->elt_size);

    array->len -= length;
    sys_array_zero_terminate(array);

    return farray;
}

SysBool sys_array_binary_search(SysArray        *array,
    const SysPointer  target,
    SysCompareFunc   compare_func,
    SysUInt         *out_match_index) {
    SysBool result = false;
    SysRealArray *_array = (SysRealArray *)array;
    SysUInt left, middle, right;
    SysInt val;

    sys_return_val_if_fail(_array != NULL, false);
    sys_return_val_if_fail(compare_func != NULL, false);

    if (_array->len) {
        left = 0;
        right = _array->len - 1;
        middle = 0;

        while (left <= right) {
            middle = left + (right - left) / 2;

            val = compare_func(_array->data + (_array->elt_size * middle), target);
            if (val == 0) {
              result = true;
              break;

            } else if (val < 0) {
              left = middle + 1;

            } else if (/* val > 0 && */ middle > 0) {
              right = middle - 1;

            } else {
              break;
            }
        }

    } else {
      middle = 0;
    }

    if (result && out_match_index != NULL)
        *out_match_index = middle;

    return result;
}

static void sys_array_maybe_expand(SysRealArray *array,
    SysUInt       len) {
    SysUInt want_alloc;

        if ((UINT_MAX - array->len) < len)
        sys_error_N("adding %u to array would overflow", len);

    want_alloc = sys_array_elt_len(array, array->len + len +
        array->zero_terminated);

    if (want_alloc > array->alloc)
    {
        want_alloc = sys_nearest_pow(want_alloc);
        want_alloc = max(want_alloc, MIN_ARRAY_SIZE);

        array->data = sys_realloc(array->data, want_alloc);
        array->alloc = want_alloc;
    }
}


typedef struct _SysRealPtrArray  SysRealPtrArray;

struct _SysRealPtrArray
{
    SysPointer       *pdata;
    SysUInt           len;
    SysUInt           alloc;
    SysRef ref_count;
    SysDestroyFunc  element_free_func;
};


static void sys_ptr_array_maybe_expand(SysRealPtrArray *array,
    SysUInt          len);

static SysPtrArray * ptr_array_new(SysUInt reserved_size,
    SysDestroyFunc element_free_func) {
    SysRealPtrArray *array;

    array = sys_slice_new(SysRealPtrArray);

    array->pdata = NULL;
    array->len = 0;
    array->alloc = 0;
    array->element_free_func = element_free_func;

    sys_block_ref_count_init(array);

    if (reserved_size != 0)
        sys_ptr_array_maybe_expand(array, reserved_size);

    return (SysPtrArray *)array;
}

SysPtrArray* sys_ptr_array_new(void) {
    return ptr_array_new(0, NULL);
}

SysPointer * sys_ptr_array_steal(SysPtrArray *array,
    SysSize *len) {
    SysRealPtrArray *rarray;
    SysPointer *segment;

    sys_return_val_if_fail(array != NULL, NULL);

    rarray = (SysRealPtrArray *)array;
    segment = (SysPointer *)rarray->pdata;

    if (len != NULL)
        *len = rarray->len;

    rarray->pdata = NULL;
    rarray->len = 0;
    rarray->alloc = 0;
    return segment;
}

SysPtrArray * sys_ptr_array_copy(SysPtrArray *array,
    SysCopyFunc  func,
    SysPointer   user_data) {
    SysPtrArray *new_array;

    sys_return_val_if_fail(array != NULL, NULL);

    new_array = ptr_array_new(array->len,
        ((SysRealPtrArray *)array)->element_free_func);

    if (func != NULL)
    {
        SysUInt i;

        for (i = 0; i < array->len; i++)
            new_array->pdata[i] = func(array->pdata[i], user_data);
    }
    else if (array->len > 0)
    {
        memcpy(new_array->pdata, array->pdata,
            array->len * sizeof(*array->pdata));
    }

    new_array->len = array->len;

    return new_array;
}

SysPtrArray* sys_ptr_array_sized_new(SysUInt reserved_size) {
    return ptr_array_new(reserved_size, NULL);
}

SysArray * sys_array_copy(SysArray *array) {
    SysRealArray *rarray = (SysRealArray *)array;
    SysRealArray *new_rarray;

    sys_return_val_if_fail(rarray != NULL, NULL);

    new_rarray =
        (SysRealArray *)sys_array_sized_new((SysBool)rarray->zero_terminated, (SysBool)rarray->clear,
            rarray->elt_size, rarray->alloc / rarray->elt_size);
    new_rarray->len = rarray->len;
    if (rarray->len > 0)
        memcpy(new_rarray->data, rarray->data, rarray->len * rarray->elt_size);

    sys_array_zero_terminate(new_rarray);

    return (SysArray *)new_rarray;
}

SysPtrArray* sys_ptr_array_new_with_free_func(SysDestroyFunc element_free_func) {
    return ptr_array_new(0, element_free_func);
}

SysPtrArray* sys_ptr_array_new_full(SysUInt          reserved_size,
    SysDestroyFunc element_free_func) {
    return ptr_array_new(reserved_size, element_free_func);
}

void sys_ptr_array_set_free_func(SysPtrArray      *array,
    SysDestroyFunc  element_free_func) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;

    sys_return_if_fail(array);

    rarray->element_free_func = element_free_func;
}

SysPtrArray* sys_ptr_array_ref(SysPtrArray *array) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;

    sys_return_val_if_fail(array, NULL);

    sys_block_ref_count_inc(rarray);

    return array;
}

static SysPointer *ptr_array_free(SysPtrArray *, ArrayFreeFlags);

void sys_ptr_array_unref(SysPtrArray *array) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;

    sys_return_if_fail(array);

    if (sys_block_ref_count_dec(rarray))
        ptr_array_free(array, FREE_SEGMENT);
}

SysPointer* sys_ptr_array_free(SysPtrArray *array,
    SysBool   free_segment) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;
    ArrayFreeFlags flags;

    sys_return_val_if_fail(rarray, NULL);

    flags = (free_segment ? FREE_SEGMENT : 0);

    /* if others are holding a reference, preserve the wrapper but
     * do free/return the data
     */
    if (!sys_block_ref_count_dec(rarray))
        flags |= PRESERVE_WRAPPER;

    return ptr_array_free(array, flags);
}

static SysPointer * ptr_array_free(SysPtrArray      *array,
    ArrayFreeFlags  flags) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;
    SysPointer *segment;

    if (flags & FREE_SEGMENT)
    {
        /* Data here is stolen and freed manually. It is an
         * error to attempt to access the array data (including
         * mutating the array bounds) during destruction).
         *
         * https://bugzilla.gnome.org/show_bug.cgi?id=769064
         */
        SysPointer *stolen_pdata = sys_steal_pointer(&rarray->pdata);
        if (rarray->element_free_func != NULL)
        {
            SysUInt i;

            for (i = 0; i < rarray->len; ++i)
                rarray->element_free_func(stolen_pdata[i]);
        }
		if (stolen_pdata != NULL) {
			sys_free(stolen_pdata);
		}
        segment = NULL;
    }
    else
        segment = rarray->pdata;

    if (flags & PRESERVE_WRAPPER)
    {
        rarray->pdata = NULL;
        rarray->len = 0;
        rarray->alloc = 0;
    }
    else
    {
        sys_slice_free1(sizeof(SysRealPtrArray), rarray);
    }

    return segment;
}

static void sys_ptr_array_maybe_expand(SysRealPtrArray *array,
    SysUInt          len) {
        if ((UINT_MAX - array->len) < len)
        sys_error_N("adding %u to array would overflow", len);

    if ((array->len + len) > array->alloc)
    {
        array->alloc = sys_nearest_pow(array->len + len);
        array->alloc = max(array->alloc, MIN_ARRAY_SIZE);
        array->pdata = sys_realloc(array->pdata, sizeof(SysPointer) * array->alloc);
    }
}

void sys_ptr_array_set_size(SysPtrArray *array,
    SysInt       length) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;
    SysUInt length_unsigned;

    sys_return_if_fail(rarray);
    sys_return_if_fail(rarray->len == 0 || (rarray->len != 0 && rarray->pdata != NULL));
    sys_return_if_fail(length >= 0);

    length_unsigned = (SysUInt)length;

    if (length_unsigned > rarray->len)
    {
        SysUInt i;
        sys_ptr_array_maybe_expand(rarray, (length_unsigned - rarray->len));
        /* This is not
         *     memset (array->pdata + array->len, 0,
         *            sizeof (SysPointer) * (length_unsigned - array->len));
         * to make it really portable. Remember (void*)NULL needn't be
         * bitwise zero. It of course is silly not to use memset (..,0,..).
         */
        for (i = rarray->len; i < length_unsigned; i++)
            rarray->pdata[i] = NULL;
    }
    else if (length_unsigned < rarray->len)
        sys_ptr_array_remove_range(array, length_unsigned, rarray->len - length_unsigned);

    rarray->len = length_unsigned;
}

static SysPointer ptr_array_remove_index(SysPtrArray *array,
    SysUInt      index_,
    SysBool   fast,
    SysBool   free_element) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;
    SysPointer result;

    sys_return_val_if_fail(rarray, NULL);
    sys_return_val_if_fail(rarray->len == 0 || (rarray->len != 0 && rarray->pdata != NULL), NULL);

    sys_return_val_if_fail(index_ < rarray->len, NULL);

    result = rarray->pdata[index_];

    if (rarray->element_free_func != NULL && free_element)
        rarray->element_free_func(rarray->pdata[index_]);

    if (index_ != rarray->len - 1 && !fast)
        memmove(rarray->pdata + index_, rarray->pdata + index_ + 1,
            sizeof(SysPointer) * (rarray->len - index_ - 1));
    else if (index_ != rarray->len - 1)
        rarray->pdata[index_] = rarray->pdata[rarray->len - 1];

    rarray->len -= 1;

    return result;
}

SysPointer sys_ptr_array_remove_index(SysPtrArray *array,
    SysUInt      index_) {
    return ptr_array_remove_index(array, index_, false, true);
}

SysPointer sys_ptr_array_remove_index_fast(SysPtrArray *array,
    SysUInt      index_) {
    return ptr_array_remove_index(array, index_, true, true);
}

SysPointer sys_ptr_array_steal_index(SysPtrArray *array,
    SysUInt      index_) {
    return ptr_array_remove_index(array, index_, false, false);
}

SysPointer sys_ptr_array_steal_index_fast(SysPtrArray *array,
    SysUInt      index_) {
    return ptr_array_remove_index(array, index_, true, false);
}

SysPtrArray* sys_ptr_array_remove_range(SysPtrArray *array,
    SysUInt      index_,
    SysUInt      length) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;
    SysUInt i;

    sys_return_val_if_fail(rarray != NULL, NULL);
    sys_return_val_if_fail(rarray->len == 0 || (rarray->len != 0 && rarray->pdata != NULL), NULL);
    sys_return_val_if_fail(index_ <= rarray->len, NULL);
    sys_return_val_if_fail(index_ + length <= rarray->len, NULL);

    if (rarray->element_free_func != NULL)
    {
        for (i = index_; i < index_ + length; i++)
            rarray->element_free_func(rarray->pdata[i]);
    }

    if (index_ + length != rarray->len)
    {
        memmove(&rarray->pdata[index_],
            &rarray->pdata[index_ + length],
            (rarray->len - (index_ + length)) * sizeof(SysPointer));
    }

    rarray->len -= length;
    
    return array;
}

SysBool sys_ptr_array_remove(SysPtrArray *array,
    SysPointer   data) {
    SysUInt i;

    sys_return_val_if_fail(array, false);
    sys_return_val_if_fail(array->len == 0 || (array->len != 0 && array->pdata != NULL), false);

    for (i = 0; i < array->len; i += 1)
    {
        if (array->pdata[i] == data)
        {
            sys_ptr_array_remove_index(array, i);
            return true;
        }
    }

    return false;
}

SysBool sys_ptr_array_remove_fast(SysPtrArray *array,
    SysPointer   data) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;
    SysUInt i;

    sys_return_val_if_fail(rarray, false);
    sys_return_val_if_fail(rarray->len == 0 || (rarray->len != 0 && rarray->pdata != NULL), false);

    for (i = 0; i < rarray->len; i += 1)
    {
        if (rarray->pdata[i] == data)
        {
            sys_ptr_array_remove_index_fast(array, i);
            return true;
        }
    }

    return false;
}

void sys_ptr_array_add(SysPtrArray *array,
    SysPointer   data) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;

    sys_return_if_fail(rarray);
    sys_return_if_fail(rarray->len == 0 || (rarray->len != 0 && rarray->pdata != NULL));

    sys_ptr_array_maybe_expand(rarray, 1);

    rarray->pdata[rarray->len++] = data;
}

void sys_ptr_array_extend(SysPtrArray  *array_to_extend,
    SysPtrArray  *array,
    SysCopyFunc   func,
    SysPointer    user_data) {
    SysRealPtrArray *rarray_to_extend = (SysRealPtrArray *)array_to_extend;

    sys_return_if_fail(array_to_extend != NULL);
    sys_return_if_fail(array != NULL);

    sys_ptr_array_maybe_expand(rarray_to_extend, array->len);

    if (func != NULL)
    {
        SysUInt i;

        for (i = 0; i < array->len; i++)
            rarray_to_extend->pdata[i + rarray_to_extend->len] =
            func(array->pdata[i], user_data);
    }
    else if (array->len > 0)
    {
        memcpy(rarray_to_extend->pdata + rarray_to_extend->len, array->pdata,
            array->len * sizeof(*array->pdata));
    }

    rarray_to_extend->len += array->len;
}

void sys_ptr_array_extend_and_steal(SysPtrArray  *array_to_extend,
    SysPtrArray  *array) {
    SysPointer *pdata;

    sys_ptr_array_extend(array_to_extend, array, NULL, NULL);

    /* Get rid of @array without triggering the SysDestroyFunc attached
     * to the elements moved from @array to @array_to_extend. */
    pdata = sys_steal_pointer(&array->pdata);
    array->len = 0;
    ((SysRealPtrArray *)array)->alloc = 0;
    sys_ptr_array_unref(array);
    sys_free(pdata);
}

void sys_ptr_array_insert(SysPtrArray *array,
    SysInt       index_,
    SysPointer   data) {
    SysRealPtrArray *rarray = (SysRealPtrArray *)array;

    sys_return_if_fail(rarray);
    sys_return_if_fail(index_ >= -1);
    sys_return_if_fail(index_ <= (SysInt)rarray->len);

    sys_ptr_array_maybe_expand(rarray, 1);

    if (index_ < 0)
        index_ = (SysInt)rarray->len;

    if ((SysUInt)index_ < rarray->len)
        memmove(&(rarray->pdata[index_ + 1]),
            &(rarray->pdata[index_]),
            (rarray->len - index_) * sizeof(SysPointer));

    rarray->len++;
    rarray->pdata[index_] = data;
}

void sys_ptr_array_foreach(SysPtrArray *array,
    SysFunc      func,
    SysPointer   user_data) {
    SysUInt i;

    sys_return_if_fail(array);

    for (i = 0; i < array->len; i++)
        (*func) (array->pdata[i], user_data);
}

SysBool sys_ptr_array_find(SysPtrArray     *haystack,
    const SysPointer  needle,
    SysUInt         *index_) {
    return sys_ptr_array_find_with_equal_func(haystack, needle, NULL, index_);
}

SysBool sys_ptr_array_find_with_equal_func(SysPtrArray     *haystack,
    const SysPointer  needle,
    SysEqualFunc     equal_func,
    SysUInt         *index_) {
  SysUInt i;

  sys_return_val_if_fail(haystack != NULL, false);

  if (equal_func == NULL)
    equal_func = sys_direct_equal;

  for (i = 0; i < haystack->len; i++)
  {
    if (equal_func(sys_ptr_array_index(haystack, i), needle))
    {
      if (index_ != NULL)
        *index_ = i;
      return true;
    }
  }

  return false;
}

SysByteArray* sys_byte_array_new(void) {
    return (SysByteArray *)sys_array_sized_new(false, false, 1, 0);
}

SysUInt8 * sys_byte_array_steal(SysByteArray *array,
    SysSize *len) {
    return (SysUInt8 *)sys_array_steal((SysArray *)array, len);
}

SysByteArray* sys_byte_array_new_take(SysUInt8 *data,
    SysSize   len) {
    SysByteArray *array;
    SysRealArray *real;

    sys_return_val_if_fail(len <= UINT_MAX, NULL);

    array = sys_byte_array_new();
    real = (SysRealArray *)array;
    sys_assert(real->data == NULL);
    sys_assert(real->len == 0);

    real->data = data;
    real->len = (SysUInt)len;
    real->alloc = (SysUInt)len;

    return array;
}

SysByteArray* sys_byte_array_sized_new(SysUInt reserved_size) {
    return (SysByteArray *)sys_array_sized_new(false, false, 1, reserved_size);
}

SysUInt8* sys_byte_array_free(SysByteArray *array,
    SysBool    free_segment) {
    return (SysUInt8 *)sys_array_free((SysArray *)array, free_segment);
}

SysByteArray* sys_byte_array_ref(SysByteArray *array) {
    return (SysByteArray *)sys_array_ref((SysArray *)array);
}

void sys_byte_array_unref(SysByteArray *array) {
    sys_array_unref((SysArray *)array);
}

SysByteArray* sys_byte_array_append(SysByteArray   *array,
    const SysUInt8 *data,
    SysUInt         len) {
    sys_array_append_vals((SysArray *)array, (SysUInt8 *)data, len);

    return array;
}

SysByteArray* sys_byte_array_prepend(SysByteArray   *array,
    const SysUInt8 *data,
    SysUInt         len) {
    sys_array_prepend_vals((SysArray *)array, (SysUInt8 *)data, len);

    return array;
}

SysByteArray* sys_byte_array_set_size(SysByteArray *array,
    SysUInt       length) {
    sys_array_set_size((SysArray *)array, length);

    return array;
}

SysByteArray* sys_byte_array_remove_index(SysByteArray *array,
    SysUInt       index_) {
    sys_array_remove_index((SysArray *)array, index_);

    return array;
}

SysByteArray* sys_byte_array_remove_index_fast(SysByteArray *array,
    SysUInt       index_) {
    sys_array_remove_index_fast((SysArray *)array, index_);

    return array;
}

SysByteArray* sys_byte_array_remove_range(SysByteArray *array,
    SysUInt       index_,
    SysUInt       length) {
    sys_return_val_if_fail(array, NULL);
    sys_return_val_if_fail(index_ <= array->len, NULL);
    sys_return_val_if_fail(index_ + length <= array->len, NULL);

    return (SysByteArray *)sys_array_remove_range((SysArray *)array, index_, length);
}
