#include <System/DataTypes/SysBytes.h>
#include <System/DataTypes/SysArray.h>
#include <System/Platform/Common/SysRefCount.h>

/**
 * SysBytes:
 *
 * A simple refcounted data type representing an immutable sequence of zero or
 * more bytes from an unspecified origin.
 *
 * The purpose of a #SysBytes is to keep the memory region that it holds
 * alive for as long as anyone holds a reference to the bytes.  When
 * the last reference count is dropped, the memory is released. Multiple
 * unrelated callers can use byte data in the #SysBytes without coordinating
 * their activities, resting assured that the byte data will not change or
 * move while they hold a reference.
 *
 * A #SysBytes can come from many different origins that may have
 * different procedures for freeing the memory region.  Examples are
 * memory from sys_malloc(), from memory slices, from a #GMappedFile or
 * memory from other allocators.
 *
 * #SysBytes work well as keys in #SysHashTable. Use sys_bytes_equal() and
 * sys_bytes_hash() as parameters to sys_hash_table_new() or sys_hash_table_new_full().
 * #SysBytes can also be used as keys in a #GTree by passing the sys_bytes_compare()
 * function to sys_tree_new().
 *
 * The data pointed to by this bytes must not be modified. For a mutable
 * array of bytes see #SysByteArray. Use sys_bytes_unref_to_array() to create a
 * mutable array for a #SysBytes sequence. To create an immutable #SysBytes from
 * a mutable #SysByteArray, use the sys_byte_array_free_to_bytes() function.
 *
 * Since: 2.32
 **/

/* Keep in sync with glib/tests/bytes.c */
struct _SysBytes
{
  SysPointer data;  /* may be NULL iff (size == 0) */
  SysSize size;  /* may be 0 */
  SysDestroyFunc free_func;
  SysPointer user_data;
  SysRef ref_count;
};

/**
 * sys_bytes_new:
 * @data: (transfer none) (array length=size) (element-type SysUInt8) (nullable):
 *        the data to be used for the bytes
 * @size: the size of @data
 *
 * Creates a new #SysBytes from @data.
 *
 * @data is copied. If @size is 0, @data may be %NULL.
 *
 * Returns: (transfer full): a new #SysBytes
 *
 * Since: 2.32
 */
SysBytes *
sys_bytes_new (const SysPointer data,
             SysSize         size)
{
  sys_return_val_if_fail (data != NULL || size == 0, NULL);

  return sys_bytes_new_take (sys_memdup2 (data, size), size);
}

/**
 * sys_bytes_new_take:
 * @data: (transfer full) (array length=size) (element-type SysUInt8) (nullable):
 *        the data to be used for the bytes
 * @size: the size of @data
 *
 * Creates a new #SysBytes from @data.
 *
 * After this call, @data belongs to the bytes and may no longer be
 * modified by the caller.  sys_free() will be called on @data when the
 * bytes is no longer in use. Because of this @data must have been created by
 * a call to sys_malloc(), sys_malloc0() or sys_realloc() or by one of the many
 * functions that wrap these calls (such as sys_new(), sys_strdup(), etc).
 *
 * For creating #SysBytes with memory from other allocators, see
 * sys_bytes_new_with_free_func().
 *
 * @data may be %NULL if @size is 0.
 *
 * Returns: (transfer full): a new #SysBytes
 *
 * Since: 2.32
 */
SysBytes * sys_bytes_new_take (SysPointer data,
                  SysSize    size) {
  return sys_bytes_new_with_free_func (data, size, sys_free, data);
}


/**
 * sys_bytes_new_static: (skip)
 * @data: (transfer full) (array length=size) (element-type SysUInt8) (nullable):
 *        the data to be used for the bytes
 * @size: the size of @data
 *
 * Creates a new #SysBytes from static data.
 *
 * @data must be static (ie: never modified or freed). It may be %NULL if @size
 * is 0.
 *
 * Returns: (transfer full): a new #SysBytes
 *
 * Since: 2.32
 */
SysBytes * sys_bytes_new_static (const SysPointer data,
    SysSize size) {

  return sys_bytes_new_with_free_func (data, size, NULL, NULL);
}

/**
 * sys_bytes_new_with_free_func: (skip)
 * @data: (array length=size) (element-type SysUInt8) (nullable):
 *        the data to be used for the bytes
 * @size: the size of @data
 * @free_func: the function to call to release the data
 * @user_data: data to pass to @free_func
 *
 * Creates a #SysBytes from @data.
 *
 * When the last reference is dropped, @free_func will be called with the
 * @user_data argument.
 *
 * @data must not be modified after this call is made until @free_func has
 * been called to indicate that the bytes is no longer in use.
 *
 * @data may be %NULL if @size is 0.
 *
 * Returns: (transfer full): a new #SysBytes
 *
 * Since: 2.32
 */
SysBytes * sys_bytes_new_with_free_func (const SysPointer  data,
                            SysSize          size,
                            SysDestroyFunc free_func,
                            SysPointer       user_data)
{
  SysBytes *bytes;

  sys_return_val_if_fail (data != NULL || size == 0, NULL);

  bytes = sys_new0 (SysBytes, 1);
  sys_ref_count_init(bytes);
  bytes->data = data;
  bytes->size = size;
  bytes->free_func = free_func;
  bytes->user_data = user_data;

  return (SysBytes *)bytes;
}

/**
 * sys_bytes_new_from_bytes:
 * @bytes: a #SysBytes
 * @offset: offset which subsection starts at
 * @length: length of subsection
 *
 * Creates a #SysBytes which is a subsection of another #SysBytes. The @offset +
 * @length may not be longer than the size of @bytes.
 *
 * A reference to @bytes will be held by the newly created #SysBytes until
 * the byte data is no longer needed.
 *
 * Since 2.56, if @offset is 0 and @length matches the size of @bytes, then
 * @bytes will be returned with the reference count incremented by 1. If @bytes
 * is a slice of another #SysBytes, then the resulting #SysBytes will reference
 * the same #SysBytes instead of @bytes. This allows consumers to simplify the
 * usage of #SysBytes when asynchronously writing to streams.
 *
 * Returns: (transfer full): a new #SysBytes
 *
 * Since: 2.32
 */
SysBytes *
sys_bytes_new_from_bytes (SysBytes  *bytes,
                        SysSize    offset,
                        SysSize    length) {
  SysChar *base;

  /* Note that length may be 0. */
  sys_return_val_if_fail (bytes != NULL, NULL);
  sys_return_val_if_fail (offset <= bytes->size, NULL);
  sys_return_val_if_fail (offset + length <= bytes->size, NULL);

  /* Avoid an extra SysBytes if all bytes were requested */
  if (offset == 0 && length == bytes->size)
    return sys_bytes_ref (bytes);

  base = (SysChar *)bytes->data + offset;

  /* Avoid referencing intermediate SysBytes. In practice, this should
   * only loop once.
   */
  while (bytes->free_func == (SysDestroyFunc)sys_bytes_unref)
    bytes = bytes->user_data;

  sys_return_val_if_fail (bytes != NULL, NULL);
  sys_return_val_if_fail (base >= (SysChar *)bytes->data, NULL);
  sys_return_val_if_fail (base <= (SysChar *)bytes->data + bytes->size, NULL);
  sys_return_val_if_fail (base + length <= (SysChar *)bytes->data + bytes->size, NULL);

  return sys_bytes_new_with_free_func (base, length,
                                     (SysDestroyFunc)sys_bytes_unref, sys_bytes_ref (bytes));
}

/**
 * sys_bytes_get_data:
 * @bytes: a #SysBytes
 * @size: (out) (optional): location to return size of byte data
 *
 * Get the byte data in the #SysBytes. This data should not be modified.
 *
 * This function will always return the same pointer for a given #SysBytes.
 *
 * %NULL may be returned if @size is 0. This is not guaranteed, as the #SysBytes
 * may represent an empty string with @data non-%NULL and @size as 0. %NULL will
 * not be returned if @size is non-zero.
 *
 * Returns: (transfer none) (array length=size) (element-type SysUInt8) (nullable):
 *          a pointer to the byte data, or %NULL
 *
 * Since: 2.32
 */
const SysPointer
sys_bytes_get_data (SysBytes *bytes,
                  SysSize *size) {
  sys_return_val_if_fail (bytes != NULL, NULL);
  if (size)
    *size = bytes->size;
  return bytes->data;
}

/**
 * sys_bytes_get_size:
 * @bytes: a #SysBytes
 *
 * Get the size of the byte data in the #SysBytes.
 *
 * This function will always return the same value for a given #SysBytes.
 *
 * Returns: the size
 *
 * Since: 2.32
 */
SysSize
sys_bytes_get_size (SysBytes *bytes)
{
  sys_return_val_if_fail (bytes != NULL, 0);
  return bytes->size;
}


/**
 * sys_bytes_ref:
 * @bytes: a #SysBytes
 *
 * Increase the reference count on @bytes.
 *
 * Returns: the #SysBytes
 *
 * Since: 2.32
 */
SysBytes *
sys_bytes_ref (SysBytes *bytes)
{
  sys_return_val_if_fail (bytes != NULL, NULL);

  sys_ref_count_inc (bytes);

  return bytes;
}

/**
 * sys_bytes_unref:
 * @bytes: (nullable): a #SysBytes
 *
 * Releases a reference on @bytes.  This may result in the bytes being
 * freed. If @bytes is %NULL, it will return immediately.
 *
 * Since: 2.32
 */
void
sys_bytes_unref (SysBytes *bytes)
{
  if (bytes == NULL)
    return;

  if (sys_ref_count_dec (bytes))
    {
      if (bytes->free_func != NULL)
        bytes->free_func (bytes->user_data);
      sys_free (bytes);
    }
}

/**
 * sys_bytes_equal:
 * @bytes1: (type GLib.Bytes): a pointer to a #SysBytes
 * @bytes2: (type GLib.Bytes): a pointer to a #SysBytes to compare with @bytes1
 *
 * Compares the two #SysBytes values being pointed to and returns
 * %true if they are equal.
 *
 * This function can be passed to sys_hash_table_new() as the @key_equal_func
 * parameter, when using non-%NULL #SysBytes pointers as keys in a #SysHashTable.
 *
 * Returns: %true if the two keys match.
 *
 * Since: 2.32
 */
SysBool
sys_bytes_equal (const SysPointer bytes1,
               const SysPointer bytes2)
{
  const SysBytes *b1 = bytes1;
  const SysBytes *b2 = bytes2;

  sys_return_val_if_fail (bytes1 != NULL, false);
  sys_return_val_if_fail (bytes2 != NULL, false);

  return b1->size == b2->size &&
         (b1->size == 0 || memcmp (b1->data, b2->data, b1->size) == 0);
}

/**
 * sys_bytes_hash:
 * @bytes: (type GLib.Bytes): a pointer to a #SysBytes key
 *
 * Creates an integer hash code for the byte data in the #SysBytes.
 *
 * This function can be passed to sys_hash_table_new() as the @key_hash_func
 * parameter, when using non-%NULL #SysBytes pointers as keys in a #SysHashTable.
 *
 * Returns: a hash value corresponding to the key.
 *
 * Since: 2.32
 */
SysUInt
sys_bytes_hash (const SysPointer bytes)
{
  const SysBytes *a = bytes;
  const signed char *p, *e;
  SysUInt32 h = 5381;

  sys_return_val_if_fail (bytes != NULL, 0);

  for (p = (signed char *)a->data, e = (signed char *)a->data + a->size; p != e; p++)
    h = (h << 5) + h + *p;

  return h;
}

/**
 * sys_bytes_compare:
 * @bytes1: (type GLib.Bytes): a pointer to a #SysBytes
 * @bytes2: (type GLib.Bytes): a pointer to a #SysBytes to compare with @bytes1
 *
 * Compares the two #SysBytes values.
 *
 * This function can be used to sort SysBytes instances in lexicographical order.
 *
 * If @bytes1 and @bytes2 have different length but the shorter one is a
 * prefix of the longer one then the shorter one is considered to be less than
 * the longer one. Otherwise the first byte where both differ is used for
 * comparison. If @bytes1 has a smaller value at that position it is
 * considered less, otherwise greater than @bytes2.
 *
 * Returns: a negative value if @bytes1 is less than @bytes2, a positive value
 *          if @bytes1 is greater than @bytes2, and zero if @bytes1 is equal to
 *          @bytes2
 *
 *
 * Since: 2.32
 */
SysInt
sys_bytes_compare (const SysPointer bytes1,
                 const SysPointer bytes2)
{
  const SysBytes *b1 = bytes1;
  const SysBytes *b2 = bytes2;
  SysInt ret;

  sys_return_val_if_fail (bytes1 != NULL, 0);
  sys_return_val_if_fail (bytes2 != NULL, 0);

  ret = memcmp (b1->data, b2->data, min (b1->size, b2->size));
  if (ret == 0 && b1->size != b2->size)
      ret = b1->size < b2->size ? -1 : 1;
  return ret;
}

static SysPointer
try_steal_and_unref (SysBytes         *bytes,
                     SysDestroyFunc  free_func,
                     SysSize          *size)
{
  SysPointer result;

  if (bytes->free_func != free_func || bytes->data == NULL ||
      bytes->user_data != bytes->data)
    return NULL;

  /* Are we the only reference? */
  if (sys_ref_count_cmp (bytes, 1))
    {
      *size = bytes->size;
      result = (SysPointer)bytes->data;
      sys_free (bytes);
      return result;
    }

  return NULL;
}


/**
 * sys_bytes_unref_to_data:
 * @bytes: (transfer full): a #SysBytes
 * @size: (out): location to place the length of the returned data
 *
 * Unreferences the bytes, and returns a pointer the same byte data
 * contents.
 *
 * As an optimization, the byte data is returned without copying if this was
 * the last reference to bytes and bytes was created with sys_bytes_new(),
 * sys_bytes_new_take() or sys_byte_array_free_to_bytes(). In all other cases the
 * data is copied.
 *
 * Returns: (transfer full) (array length=size) (element-type SysUInt8)
 *          (not nullable): a pointer to the same byte data, which should be
 *          freed with sys_free()
 *
 * Since: 2.32
 */
SysPointer
sys_bytes_unref_to_data (SysBytes *bytes,
                       SysSize  *size) {
  SysPointer result;

  sys_return_val_if_fail (bytes != NULL, NULL);
  sys_return_val_if_fail (size != NULL, NULL);

  /*
   * Optimal path: if this is was the last reference, then we can return
   * the data from this SysBytes without copying.
   */

  result = try_steal_and_unref (bytes, sys_free, size);
  if (result == NULL)
    {
      /*
       * Copy: Non sys_malloc (or compatible) allocator, or static memory,
       * so we have to copy, and then unref.
       */
      result = sys_memdup2 (bytes->data, bytes->size);
      *size = bytes->size;
      sys_bytes_unref (bytes);
    }

  return result;
}

/**
 * sys_bytes_unref_to_array:
 * @bytes: (transfer full): a #SysBytes
 *
 * Unreferences the bytes, and returns a new mutable #SysByteArray containing
 * the same byte data.
 *
 * As an optimization, the byte data is transferred to the array without copying
 * if this was the last reference to bytes and bytes was created with
 * sys_bytes_new(), sys_bytes_new_take() or sys_byte_array_free_to_bytes(). In all
 * other cases the data is copied.
 *
 * Do not use it if @bytes contains more than %sys_MAXUINT
 * bytes. #SysByteArray stores the length of its data in #SysUInt, which
 * may be shorter than #SysSize, that @bytes is using.
 *
 * Returns: (transfer full): a new mutable #SysByteArray containing the same byte data
 *
 * Since: 2.32
 */
SysByteArray *
sys_bytes_unref_to_array (SysBytes *bytes)
{
  SysPointer data;
  SysSize size;

  sys_return_val_if_fail (bytes != NULL, NULL);

  data = sys_bytes_unref_to_data (bytes, &size);

  return sys_byte_array_new_take (data, size);
}

/**
 * sys_bytes_get_region:
 * @bytes: a #SysBytes
 * @element_size: a non-zero element size
 * @offset: an offset to the start of the region within the @bytes
 * @n_elements: the number of elements in the region
 *
 * Gets a pointer to a region in @bytes.
 *
 * The region starts at @offset many bytes from the start of the data
 * and contains @n_elements many elements of @element_size size.
 *
 * @n_elements may be zero, but @element_size must always be non-zero.
 * Ideally, @element_size is a static constant (eg: sizeof a struct).
 *
 * This function does careful bounds checking (including checking for
 * arithmetic overflows) and returns a non-%NULL pointer if the
 * specified region lies entirely within the @bytes. If the region is
 * in some way out of range, or if an overflow has occurred, then %NULL
 * is returned.
 *
 * Note: it is possible to have a valid zero-size region. In this case,
 * the returned pointer will be equal to the base pointer of the data of
 * @bytes, plus @offset.  This will be non-%NULL except for the case
 * where @bytes itself was a zero-sized region.  Since it is unlikely
 * that you will be using this function to check for a zero-sized region
 * in a zero-sized @bytes, %NULL effectively always means "error".
 *
 * Returns: (nullable): the requested region, or %NULL in case of an error
 *
 * Since: 2.70
 */
const SysPointer sys_bytes_get_region (SysBytes *bytes,
                    SysSize   element_size,
                    SysSize   offset,
                    SysSize   n_elements)
{
  SysSize total_size;
  SysSize end_offset;

  sys_return_val_if_fail (element_size > 0, NULL);

  /* No other assertion checks here.  If something is wrong then we will
   * simply crash (via NULL dereference or divide-by-zero).
   */

  if (!sys_size_checked_mul (&total_size, element_size, n_elements))
    return NULL;

  if (!sys_size_checked_add (&end_offset, offset, total_size))
    return NULL;

  /* We now have:
   *
   *   0 <= offset <= end_offset
   *
   * So we need only check that end_offset is within the range of the
   * size of @bytes and we're good to go.
   */

  if (end_offset > bytes->size) {

    return NULL;
  }

  /* We now have:
   *
   *   0 <= offset <= end_offset <= bytes->size
   */

  return ((SysUChar *) bytes->data) + offset;
}
