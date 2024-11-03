#include <System/DataTypes/SysString.h>
#include <System/DataTypes/SysBytes.h>
#include <System/Utils/SysStr.h>

/**
 * this code from glib slist
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

static void sys_string_expand (SysString *string,
    SysSize    len) {
  /* Detect potential overflow */
  if SYS_UNLIKELY ((SYS_MAXSIZE - string->len - 1) < len)
    sys_error_N ("adding %ul to string would overflow", len);

  string->allocated_len = sys_nearest_pow (string->len + len + 1);
  /* If the new size is bigger than SYS_MAXSIZE / 2, only allocate enough
   * memory for this string and don't over-allocate.
   */
  if (string->allocated_len == 0)
    string->allocated_len = string->len + len + 1;

  string->str = sys_realloc (string->str, string->allocated_len);
}

static inline void sys_string_maybe_expand (SysString *string,
    SysSize    len) {
  if (SYS_UNLIKELY (string->len + len >= string->allocated_len))
    sys_string_expand (string, len);
}

/**
 * sys_string_sized_new: (constructor)
 * @dfl_size: the default size of the space allocated to hold the string
 *
 * Creates a new #SysString, with enough space for @dfl_size
 * bytes. This is useful if you are going to add a lot of
 * text to the string and don't want it to be reallocated
 * too often.
 *
 * Returns: (transfer full): the new #SysString
 */
SysString * sys_string_sized_new (SysSize dfl_size) {
  SysString *string = sys_slice_new (SysString);

  string->allocated_len = 0;
  string->len   = 0;
  string->str   = NULL;

  sys_string_expand (string, max (dfl_size, 64));
  string->str[0] = 0;

  return string;
}

/**
 * sys_string_new: (constructor)
 * @init: (nullable): the initial text to copy into the string, or %NULL to
 *   start with an empty string
 *
 * Creates a new #SysString, initialized with the given string.
 *
 * Returns: (transfer full): the new #SysString
 */
SysString * sys_string_new (const SysChar *init) {
  SysString *string;

  if (init == NULL || *init == '\0') {
    string = sys_string_sized_new (2);

  } else {
    SysInt len;

    len = strlen (init);
    string = sys_string_sized_new (len + 2);

    sys_string_append_len (string, init, len);
  }

  return string;
}

/**
 * sys_string_new_take: (constructor)
 * @init: (nullable) (transfer full): initial text used as the string.
 *     Ownership of the string is transferred to the #SysString.
 *     Passing %NULL creates an empty string.
 *
 * Creates a new #SysString, initialized with the given string.
 *
 * After this call, @init belongs to the #SysString and may no longer be
 * modified by the caller. The memory of @data has to be dynamically
 * allocated and will eventually be freed with sys_free().
 *
 * Returns: (transfer full): the new #SysString
 *
 * Since: 2.78
 */
SysString * sys_string_new_take (SysChar *init) {
  SysString *string;

  if (init == NULL) {

    return sys_string_new (NULL);
  }

  string = sys_slice_new (SysString);

  string->str = init;
  string->len = strlen (string->str);
  string->allocated_len = string->len + 1;

  return string;
}

/**
 * sys_string_new_len: (constructor)
 * @init: initial contents of the string
 * @len: length of @init to use
 *
 * Creates a new #SysString with @len bytes of the @init buffer.
 * Because a length is provided, @init need not be nul-terminated,
 * and can contain embedded nul bytes.
 *
 * Since this function does not stop at nul bytes, it is the caller's
 * responsibility to ensure that @init has at least @len addressable
 * bytes.
 *
 * Returns: (transfer full): a new #SysString
 */
SysString *sys_string_new_len (const SysChar *init,
    SysSize       len) {
  SysString *string;

  if (len < 0) {
    return sys_string_new (init);
  }
  else
  {
    string = sys_string_sized_new (len);

    if (init) {

      sys_string_append_len (string, init, len);
    }

    return string;
  }
}

/**
 * sys_string_free:
 * @string: (transfer full): a #SysString
 * @free_segment: if %true, the actual character data is freed as well
 *
 * Frees the memory allocated for the #SysString.
 * If @free_segment is %true it also frees the character data.  If
 * it's %false, the caller gains ownership of the buffer and must
 * free it after use with sys_free().
 *
 * Instead of passing %false to this function, consider using
 * sys_string_free_and_steal().
 *
 * Returns: (nullable): the character data of @string
 *          (i.e. %NULL if @free_segment is %true)
 */
SysChar * sys_string_free (SysString  *string,
    SysBool  free_segment) {
  SysChar *segment;

  sys_return_val_if_fail (string != NULL, NULL);

  if (free_segment)
  {
    sys_free (string->str);
    segment = NULL;
  }
  else
    segment = string->str;

  sys_slice_free (SysString, string);

  return segment;
}

/**
 * sys_string_free_and_steal:
 * @string: (transfer full): a #SysString
 *
 * Frees the memory allocated for the #SysString.
 *
 * The caller gains ownership of the buffer and
 * must free it after use with sys_free().
 *
 * Returns: (transfer full): the character data of @string
 *
 * Since: 2.76
 */
SysChar * sys_string_free_and_steal (SysString *string) {

  return sys_string_free (string, false);
}

/**
 * sys_string_free_to_bytes:
 * @string: (transfer full): a #SysString
 *
 * Transfers ownership of the contents of @string to a newly allocated
 * #SysBytes.  The #SysString structure itself is deallocated, and it is
 * therefore invalid to use @string after invoking this function.
 *
 * Note that while #SysString ensures that its buffer always has a
 * trailing nul character (not reflected in its "len"), the returned
 * #SysBytes does not include this extra nul; i.e. it has length exactly
 * equal to the "len" member.
 *
 * Returns: (transfer full): A newly allocated #SysBytes containing contents of @string; @string itself is freed
 * Since: 2.34
 */
SysBytes* sys_string_free_to_bytes (SysString *string) {
  SysSize len;
  SysChar *buf;

  sys_return_val_if_fail (string != NULL, NULL);

  len = string->len;

  buf = sys_string_free (string, false);

  return sys_bytes_new_take (buf, len);
}

/**
 * sys_string_equal:
 * @v: a #SysString
 * @v2: another #SysString
 *
 * Compares two strings for equality, returning %true if they are equal.
 * For use with #SysHashTable.
 *
 * Returns: %true if the strings are the same length and contain the
 *     same bytes
 */
SysBool sys_string_equal (const SysString *v,
    const SysString *v2) {
  SysChar *p, *q;
  SysString *string1 = (SysString *) v;
  SysString *string2 = (SysString *) v2;
  SysSize i = string1->len;

  if (i != string2->len)
    return false;

  p = string1->str;
  q = string2->str;
  while (i)
  {
    if (*p != *q)
      return false;
    p++;
    q++;
    i--;
  }
  return true;
}

/**
 * sys_string_hash:
 * @str: a string to hash
 *
 * Creates a hash code for @str; for use with #SysHashTable.
 *
 * Returns: hash code for @str
 */
SysUInt sys_string_hash (const SysString *str) {
  const SysChar *p = str->str;
  SysSize n = str->len;
  SysUInt h = 0;

  /* 31 bit hash function */
  while (n--)
  {
    h = (h << 5) - h + *p;
    p++;
  }

  return h;
}

/**
 * sys_string_assign:
 * @string: the destination #SysString. Its current contents
 *          are destroyed.
 * @rval: the string to copy into @string
 *
 * Copies the bytes from a string into a #SysString,
 * destroying any previous contents. It is rather like
 * the standard strcpy() function, except that you do not
 * have to worry about having enough space to copy the string.
 *
 * Returns: (transfer none): @string
 */
SysString *sys_string_assign (SysString *string,
    const SysChar *rval) {
  sys_return_val_if_fail (string != NULL, NULL);
  sys_return_val_if_fail (rval != NULL, string);

  /* Make sure assigning to itself doesn't corrupt the string. */
  if (string->str != rval)
  {
    /* Assigning from substring should be ok, since
     * sys_string_truncate() does not reallocate.
     */
    sys_string_truncate (string, 0);
    sys_string_append (string, rval);
  }

  return string;
}

/**
 * sys_string_truncate:
 * @string: a #SysString
 * @len: the new size of @string
 *
 * Cuts off the end of the SysString, leaving the first @len bytes.
 *
 * Returns: (transfer none): @string
 */
SysString *sys_string_truncate (SysString *string,
    SysSize len) {
  sys_return_val_if_fail (string != NULL, NULL);

  string->len = min (len, string->len);
  string->str[string->len] = 0;

  return string;
}

/**
 * sys_string_set_size:
 * @string: a #SysString
 * @len: the new length
 *
 * Sets the length of a #SysString. If the length is less than
 * the current length, the string will be truncated. If the
 * length is greater than the current length, the contents
 * of the newly added area are undefined. (However, as
 * always, string->str[string->len] will be a nul byte.)
 *
 * Returns: (transfer none): @string
 */
SysString *sys_string_set_size (SysString *string,
    SysSize    len) {
  sys_return_val_if_fail (string != NULL, NULL);

  if (len >= string->allocated_len)
    sys_string_maybe_expand (string, len - string->len);

  string->len = len;
  string->str[len] = 0;

  return string;
}

/**
 * sys_string_insert_len:
 * @string: a #SysString
 * @pos: position in @string where insertion should
 *       happen, or -1 for at the end
 * @val: bytes to insert
 * @len: number of bytes of @val to insert, or -1 for all of @val
 *
 * Inserts @len bytes of @val into @string at @pos.
 *
 * If @len is positive, @val may contain embedded nuls and need
 * not be nul-terminated. It is the caller's responsibility to
 * ensure that @val has at least @len addressable bytes.
 *
 * If @len is negative, @val must be nul-terminated and @len
 * is considered to request the entire string length.
 *
 * If @pos is -1, bytes are inserted at the end of the string.
 *
 * Returns: (transfer none): @string
 */
SysString *sys_string_insert_len (SysString     *string,
    SysSize       pos,
    const SysChar *val,
    SysSize       len) {
  SysSize len_unsigned, pos_unsigned;

  sys_return_val_if_fail (string != NULL, NULL);
  sys_return_val_if_fail (len == 0 || val != NULL, string);

  if (len == 0) {

    return string;
  }

  if (len < 0) {

    len = strlen (val);
  }
  len_unsigned = len;

  if (pos < 0) {

    pos_unsigned = string->len;

  } else {

    pos_unsigned = pos;
    sys_return_val_if_fail (pos_unsigned <= string->len, string);
  }

  /* Check whether val represents a substring of string.
   * This test probably violates chapter and verse of the C standards,
   * since ">=" and "<=" are only valid when val really is a substring.
   * In practice, it will work on modern archs.
   */
  if (SYS_UNLIKELY (val >= string->str && val <= string->str + string->len))
  {
    SysSize offset = val - string->str;
    SysSize precount = 0;

    sys_string_maybe_expand (string, len_unsigned);
    val = string->str + offset;
    /* At this point, val is valid again.  */

    /* Open up space where we are going to insert.  */
    if (pos_unsigned < string->len) {

      memmove (string->str + pos_unsigned + len_unsigned,
          string->str + pos_unsigned, string->len - pos_unsigned);
    }

    /* Move the source part before the gap, if any.  */
    if (offset < pos_unsigned)
    {
      precount = min (len_unsigned, pos_unsigned - offset);
      memcpy (string->str + pos_unsigned, val, precount);
    }

    /* Move the source part after the gap, if any.  */
    if (len_unsigned > precount)
    {
      sys_memcpy (string->str + pos_unsigned + precount,
          string->allocated_len,
          val + /* Already moved: */ precount +
          /* Space opened up: */ len_unsigned,
          len_unsigned - precount);
    }

  } else {
    sys_string_maybe_expand (string, len_unsigned);

    /* If we aren't appending at the end, move a hunk
     * of the old string to the end, opening up space
     */
    if (pos_unsigned < string->len) {

      memmove (string->str + pos_unsigned + len_unsigned,
          string->str + pos_unsigned, string->len - pos_unsigned);
    }

    /* insert the new string */
    if (len_unsigned == 1) {

      string->str[pos_unsigned] = *val;

    } else {

      sys_memcpy (string->str + pos_unsigned, string->allocated_len, val, len_unsigned);
    }
  }

  string->len += len_unsigned;

  string->str[string->len] = 0;

  return string;
}

/**
 * sys_string_append:
 * @string: a #SysString
 * @val: the string to append onto the end of @string
 *
 * Adds a string onto the end of a #SysString, expanding
 * it if necessary.
 *
 * Returns: (transfer none): @string
 */
SysString *sys_string_append (SysString     *string,
    const SysChar *val) {

  return sys_string_insert_len (string, -1, val, -1);
}

/**
 * sys_string_append_len:
 * @string: a #SysString
 * @val: bytes to append
 * @len: number of bytes of @val to use, or -1 for all of @val
 *
 * Appends @len bytes of @val to @string.
 *
 * If @len is positive, @val may contain embedded nuls and need
 * not be nul-terminated. It is the caller's responsibility to
 * ensure that @val has at least @len addressable bytes.
 *
 * If @len is negative, @val must be nul-terminated and @len
 * is considered to request the entire string length. This
 * makes sys_string_append_len() equivalent to sys_string_append().
 *
 * Returns: (transfer none): @string
 */
SysString *sys_string_append_len (SysString     *string,
    const SysChar *val,
    SysSize       len) {
  return sys_string_insert_len (string, -1, val, len);
}

/**
 * sys_string_append_c:
 * @string: a #SysString
 * @c: the byte to append onto the end of @string
 *
 * Adds a byte onto the end of a #SysString, expanding
 * it if necessary.
 *
 * Returns: (transfer none): @string
 */
SysString *              (sys_string_append_c) (SysString *string,
    SysChar    c) {
  sys_return_val_if_fail (string != NULL, NULL);

  return sys_string_insert_c (string, -1, c);
}

/**
 * sys_string_append_unichar:
 * @string: a #SysString
 * @wc: a Unicode character
 *
 * Converts a Unicode character into UTF-8, and appends it
 * to the string.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_append_unichar (SysString  *string,
    SysUniChar  wc) {
  sys_return_val_if_fail (string != NULL, NULL);

  return sys_string_insert_unichar (string, -1, wc);
}

/**
 * sys_string_prepend:
 * @string: a #SysString
 * @val: the string to prepend on the start of @string
 *
 * Adds a string on to the start of a #SysString,
 * expanding it if necessary.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_prepend (SysString     *string,
    const SysChar *val) {
  return sys_string_insert_len (string, 0, val, -1);
}

/**
 * sys_string_prepend_len:
 * @string: a #SysString
 * @val: bytes to prepend
 * @len: number of bytes in @val to prepend, or -1 for all of @val
 *
 * Prepends @len bytes of @val to @string.
 *
 * If @len is positive, @val may contain embedded nuls and need
 * not be nul-terminated. It is the caller's responsibility to
 * ensure that @val has at least @len addressable bytes.
 *
 * If @len is negative, @val must be nul-terminated and @len
 * is considered to request the entire string length. This
 * makes sys_string_prepend_len() equivalent to sys_string_prepend().
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_prepend_len (SysString     *string,
    const SysChar *val,
    SysSize       len) {
  return sys_string_insert_len (string, 0, val, len);
}

/**
 * sys_string_prepend_c:
 * @string: a #SysString
 * @c: the byte to prepend on the start of the #SysString
 *
 * Adds a byte onto the start of a #SysString,
 * expanding it if necessary.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_prepend_c (SysString *string,
    SysChar    c) {
  sys_return_val_if_fail (string != NULL, NULL);

  return sys_string_insert_c (string, 0, c);
}

/**
 * sys_string_prepend_unichar:
 * @string: a #SysString
 * @wc: a Unicode character
 *
 * Converts a Unicode character into UTF-8, and prepends it
 * to the string.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_prepend_unichar (SysString  *string,
    SysUniChar  wc) {
  sys_return_val_if_fail (string != NULL, NULL);

  return sys_string_insert_unichar (string, 0, wc);
}

/**
 * sys_string_insert:
 * @string: a #SysString
 * @pos: the position to insert the copy of the string
 * @val: the string to insert
 *
 * Inserts a copy of a string into a #SysString,
 * expanding it if necessary.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_insert (SysString     *string,
    SysSize       pos,
    const SysChar *val) {
  return sys_string_insert_len (string, pos, val, -1);
}

/**
 * sys_string_insert_c:
 * @string: a #SysString
 * @pos: the position to insert the byte
 * @c: the byte to insert
 *
 * Inserts a byte into a #SysString, expanding it if necessary.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_insert_c (SysString *string,
    SysSize   pos,
    SysChar    c) {
  SysSize pos_unsigned;

  sys_return_val_if_fail (string != NULL, NULL);

  sys_string_maybe_expand (string, 1);

  if (pos < 0)
    pos = string->len;
  else
    sys_return_val_if_fail ((SysSize) pos <= string->len, string);
  pos_unsigned = pos;

  /* If not just an append, move the old stuff */
  if (pos_unsigned < string->len)
    memmove (string->str + pos_unsigned + 1,
        string->str + pos_unsigned, string->len - pos_unsigned);

  string->str[pos_unsigned] = c;

  string->len += 1;

  string->str[string->len] = 0;

  return string;
}

/**
 * sys_string_insert_unichar:
 * @string: a #SysString
 * @pos: the position at which to insert character, or -1
 *     to append at the end of the string
 * @wc: a Unicode character
 *
 * Converts a Unicode character into UTF-8, and insert it
 * into the string at the given position.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_insert_unichar (SysString  *string,
    SysSize    pos,
    SysUniChar  wc) {
  SysInt charlen, first, i;
  SysChar *dest;

  sys_return_val_if_fail (string != NULL, NULL);

  /* Code copied from sys_unichar_to_utf() */
  if (wc < 0x80) {
    first = 0;
    charlen = 1;

  } else if (wc < 0x800) {
    first = 0xc0;
    charlen = 2;

  } else if (wc < 0x10000) {

    first = 0xe0;
    charlen = 3;

  } else if (wc < 0x200000) {
    first = 0xf0;
    charlen = 4;

  } else if (wc < 0x4000000) {
    first = 0xf8;
    charlen = 5;

  } else {

    first = 0xfc;
    charlen = 6;
  }
  /* End of copied code */

  sys_string_maybe_expand (string, charlen);

  if (pos < 0) {

    pos = string->len;

  } else {

    sys_return_val_if_fail ((SysSize) pos <= string->len, string);
  }

  /* If not just an append, move the old stuff */
  if ((SysSize) pos < string->len) {

    memmove (string->str + pos + charlen, string->str + pos, string->len - pos);
  }

  dest = string->str + pos;
  /* Code copied from sys_unichar_to_utf() */
  for (i = charlen - 1; i > 0; --i)
  {
    dest[i] = (wc & 0x3f) | 0x80;
    wc >>= 6;
  }
  dest[0] = wc | first;
  /* End of copied code */

  string->len += charlen;

  string->str[string->len] = 0;

  return string;
}

/**
 * sys_string_overwrite:
 * @string: a #SysString
 * @pos: the position at which to start overwriting
 * @val: the string that will overwrite the @string starting at @pos
 *
 * Overwrites part of a string, lengthening it if necessary.
 *
 * Returns: (transfer none): @string
 *
 * Since: 2.14
 */
SysString *              sys_string_overwrite (SysString     *string,
    SysSize        pos,
    const SysChar *val) {
  sys_return_val_if_fail (val != NULL, string);
  return sys_string_overwrite_len (string, pos, val, strlen (val));
}

/**
 * sys_string_overwrite_len:
 * @string: a #SysString
 * @pos: the position at which to start overwriting
 * @val: the string that will overwrite the @string starting at @pos
 * @len: the number of bytes to write from @val
 *
 * Overwrites part of a string, lengthening it if necessary.
 * This function will work with embedded nuls.
 *
 * Returns: (transfer none): @string
 *
 * Since: 2.14
 */
SysString *              sys_string_overwrite_len (SysString     *string,
    SysSize        pos,
    const SysChar *val,
    SysSize       len) {
  SysSize end;

  sys_return_val_if_fail (string != NULL, NULL);

  if (!len)
    return string;

  sys_return_val_if_fail (val != NULL, string);
  sys_return_val_if_fail (pos <= string->len, string);

  if (len < 0)
    len = strlen (val);

  end = pos + len;

  if (end > string->len)
    sys_string_maybe_expand (string, end - string->len);

  memcpy (string->str + pos, val, len);

  if (end > string->len)
  {
    string->str[end] = '\0';
    string->len = end;
  }

  return string;
}

/**
 * sys_string_erase:
 * @string: a #SysString
 * @pos: the position of the content to remove
 * @len: the number of bytes to remove, or -1 to remove all
 *       following bytes
 *
 * Removes @len bytes from a #SysString, starting at position @pos.
 * The rest of the #SysString is shifted down to fill the gap.
 *
 * Returns: (transfer none): @string
 */
SysString *              sys_string_erase (SysString *string,
    SysSize   pos,
    SysSize   len) {
  SysSize len_unsigned, pos_unsigned;

  sys_return_val_if_fail (string != NULL, NULL);
  sys_return_val_if_fail (pos >= 0, string);
  pos_unsigned = pos;

  sys_return_val_if_fail (pos_unsigned <= string->len, string);

  if (len < 0)
    len_unsigned = string->len - pos_unsigned;
  else
  {
    len_unsigned = len;
    sys_return_val_if_fail (pos_unsigned + len_unsigned <= string->len, string);

    if (pos_unsigned + len_unsigned < string->len)
      memmove (string->str + pos_unsigned,
          string->str + pos_unsigned + len_unsigned,
          string->len - (pos_unsigned + len_unsigned));
  }

  string->len -= len_unsigned;

  string->str[string->len] = 0;

  return string;
}

/**
 * sys_string_replace:
 * @string: a #SysString
 * @find: the string to find in @string
 * @replace: the string to insert in place of @find
 * @limit: the maximum instances of @find to replace with @replace, or `0` for
 * no limit
 *
 * Replaces the string @find with the string @replace in a #SysString up to
 * @limit times. If the number of instances of @find in the #SysString is
 * less than @limit, all instances are replaced. If @limit is `0`,
 * all instances of @find are replaced.
 *
 * If @find is the empty string, since versions 2.69.1 and 2.68.4 the
 * replacement will be inserted no more than once per possible position
 * (beginning of string, end of string and between characters). This did
 * not work correctly in earlier versions.
 *
 * Returns: the number of find and replace operations performed.
 *
 * Since: 2.68
 */
SysUInt
sys_string_replace (SysString     *string,
    const SysChar *find,
    const SysChar *replace,
    SysUInt        limit) {
  SysSize f_len, r_len, pos;
  SysChar *cur, *next;
  SysUInt n = 0;

  sys_return_val_if_fail (string != NULL, 0);
  sys_return_val_if_fail (find != NULL, 0);
  sys_return_val_if_fail (replace != NULL, 0);

  f_len = strlen (find);
  r_len = strlen (replace);
  cur = string->str;

  while ((next = strstr (cur, find)) != NULL)
  {
    pos = next - string->str;
    sys_string_erase (string, pos, f_len);
    sys_string_insert (string, pos, replace);
    cur = string->str + pos + r_len;
    n++;
    /* Only match the empty string once at any given position, to
     * avoid infinite loops */
    if (f_len == 0)
    {
      if (cur[0] == '\0')
        break;
      else
        cur++;
    }
    if (n == limit)
      break;
  }

  return n;
}

/**
 * sys_string_ascii_down:
 * @string: a SysString
 *
 * Converts all uppercase ASCII letters to lowercase ASCII letters.
 *
 * Returns: (transfer none): passed-in @string pointer, with all the
 *     uppercase characters converted to lowercase in place,
 *     with semantics that exactly match sys_ascii_tolower().
 */
SysString *sys_string_ascii_down (SysString *string) {
  SysChar *s;
  SysInt n;

  sys_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = string->str;

  while (n) {

    *s = tolower (*s);
    s++;
    n--;
  }

  return string;
}

/**
 * sys_string_ascii_up:
 * @string: a SysString
 *
 * Converts all lowercase ASCII letters to uppercase ASCII letters.
 *
 * Returns: (transfer none): passed-in @string pointer, with all the
 *     lowercase characters converted to uppercase in place,
 *     with semantics that exactly match sys_ascii_toupper().
 */
SysString *sys_string_ascii_up (SysString *string) {
  SysChar *s;
  SysInt n;

  sys_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = string->str;

  while (n) {
    *s = tolower (*s);
    s++;
    n--;
  }

  return string;
}

/**
 * sys_string_down:
 * @string: a #SysString
 *
 * Converts a #SysString to lowercase.
 *
 * Returns: (transfer none): the #SysString
 *
 * Deprecated:2.2: This function uses the locale-specific
 *     tolower() function, which is almost never the right thing.
 *     Use sys_string_ascii_down() or sys_utf8_strdown() instead.
 */
SysString *sys_string_down (SysString *string) {
  SysUChar *s;
  SysLong n;

  sys_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = (SysUChar *) string->str;

  while (n)
  {
    if (isupper (*s))
      *s = tolower (*s);
    s++;
    n--;
  }

  return string;
}

/**
 * sys_string_up:
 * @string: a #SysString
 *
 * Converts a #SysString to uppercase.
 *
 * Returns: (transfer none): @string
 *
 * Deprecated:2.2: This function uses the locale-specific
 *     toupper() function, which is almost never the right thing.
 *     Use sys_string_ascii_up() or sys_utf8_strup() instead.
 */
SysString * sys_string_up (SysString *string) {
  SysUChar *s;
  SysLong n;

  sys_return_val_if_fail (string != NULL, NULL);

  n = string->len;
  s = (SysUChar *) string->str;

  while (n)
  {
    if (islower (*s))
      *s = toupper (*s);
    s++;
    n--;
  }

  return string;
}

/**
 * sys_string_append_vprintf:
 * @string: a #SysString
 * @format: (not nullable): the string format. See the printf() documentation
 * @args: the list of arguments to insert in the output
 *
 * Appends a formatted string onto the end of a #SysString.
 * This function is similar to sys_string_append_printf()
 * except that the arguments to the format string are passed
 * as a va_list.
 *
 * Since: 2.14
 */
void sys_string_append_vprintf (SysString     *string,
    const SysChar *format,
    va_list      args) {
  SysChar *buf;
  SysInt len;

  sys_return_if_fail (string != NULL);
  sys_return_if_fail (format != NULL);

  len = sys_vasprintf (&buf, format, args);

  if (len >= 0)
  {
    sys_string_maybe_expand (string, len);
    memcpy (string->str + string->len, buf, len + 1);
    string->len += len;
    sys_free (buf);
  }
}

/**
 * sys_string_vprintf:
 * @string: a #SysString
 * @format: (not nullable): the string format. See the printf() documentation
 * @args: the parameters to insert into the format string
 *
 * Writes a formatted string into a #SysString.
 * This function is similar to sys_string_printf() except that
 * the arguments to the format string are passed as a va_list.
 *
 * Since: 2.14
 */
void sys_string_vprintf (SysString     *string,
    const SysChar *format,
    va_list      args) {
  sys_string_truncate (string, 0);
  sys_string_append_vprintf (string, format, args);
}

/**
 * sys_string_sprintf:
 * @string: a #SysString
 * @format: the string format. See the sprintf() documentation
 * @...: the parameters to insert into the format string
 *
 * Writes a formatted string into a #SysString.
 * This is similar to the standard sprintf() function,
 * except that the #SysString buffer automatically expands
 * to contain the results. The previous contents of the
 * #SysString are destroyed.
 *
 * Deprecated: This function has been renamed to sys_string_printf().
 */

/**
 * sys_string_printf:
 * @string: a #SysString
 * @format: the string format. See the printf() documentation
 * @...: the parameters to insert into the format string
 *
 * Writes a formatted string into a #SysString.
 * This is similar to the standard sprintf() function,
 * except that the #SysString buffer automatically expands
 * to contain the results. The previous contents of the
 * #SysString are destroyed.
 */
void sys_string_printf (SysString     *string,
    const SysChar *format,
    ...) {
  va_list args;

  sys_string_truncate (string, 0);

  va_start (args, format);
  sys_string_append_vprintf (string, format, args);
  va_end (args);
}

/**
 * sys_string_sprintfa:
 * @string: a #SysString
 * @format: the string format. See the sprintf() documentation
 * @...: the parameters to insert into the format string
 *
 * Appends a formatted string onto the end of a #SysString.
 * This function is similar to sys_string_sprintf() except that
 * the text is appended to the #SysString.
 *
 * Deprecated: This function has been renamed to sys_string_append_printf()
 */

/**
 * sys_string_append_printf:
 * @string: a #SysString
 * @format: the string format. See the printf() documentation
 * @...: the parameters to insert into the format string
 *
 * Appends a formatted string onto the end of a #SysString.
 * This function is similar to sys_string_printf() except
 * that the text is appended to the #SysString.
 */
void sys_string_append_printf (SysString     *string,
    const SysChar *format,
    ...) {
  va_list args;

  va_start (args, format);
  sys_string_append_vprintf (string, format, args);
  va_end (args);
}
