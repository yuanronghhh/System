#include <System/Utils/SysUtf8.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysUnicode.h>
#include <System/Utils/SysUnicodeTable.h>
#include <System/DataTypes/SysString.h>

#define UTF8_COMPUTE(Char, Mask, Len)                                              \
  if (Char < 128)                                                              \
    {                                                                              \
      Len = 1;                                                                      \
      Mask = 0x7f;                                                              \
    }                                                                              \
  else if ((Char & 0xe0) == 0xc0)                                              \
    {                                                                              \
      Len = 2;                                                                      \
      Mask = 0x1f;                                                              \
    }                                                                              \
  else if ((Char & 0xf0) == 0xe0)                                              \
    {                                                                              \
      Len = 3;                                                                      \
      Mask = 0x0f;                                                              \
    }                                                                              \
  else if ((Char & 0xf8) == 0xf0)                                              \
    {                                                                              \
      Len = 4;                                                                      \
      Mask = 0x07;                                                              \
    }                                                                              \
  else if ((Char & 0xfc) == 0xf8)                                              \
    {                                                                              \
      Len = 5;                                                                      \
      Mask = 0x03;                                                              \
    }                                                                              \
  else if ((Char & 0xfe) == 0xfc)                                              \
    {                                                                              \
      Len = 6;                                                                      \
      Mask = 0x01;                                                              \
    }                                                                              \
  else                                                                              \
    Len = -1;

#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))

#define UTF8_GET(Result, Chars, Count, Mask, Len)                              \
  (Result) = (Chars)[0] & (Mask);                                              \
  for ((Count) = 1; (Count) < (Len); ++(Count))                                      \
    {                                                                              \
      if (((Chars)[(Count)] & 0xc0) != 0x80)                                      \
        {                                                                      \
          (Result) = -1;                                                      \
          break;                                                              \
        }                                                                      \
      (Result) <<= 6;                                                              \
      (Result) |= ((Chars)[(Count)] & 0x3f);                                      \
    }

/*
 * Check whether a Unicode (5.2) char is in a valid range.
 *
 * The first check comes from the Unicode guarantee to never encode
 * a point above 0x0010ffff, since UTF-16 couldn't represent it.
 *
 * The second check covers surrogate pairs (category Cs).
 *
 * @param Char the character
 */
#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000 &&                     \
     (((Char) & 0xFFFFF800) != 0xD800))


static const SysChar utf8_skip_data[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

const SysChar * const sys_utf8_skip = utf8_skip_data;

/**
 * sys_utf8_find_prev_char:
 * @str: pointer to the beginning of a UTF-8 encoded string
 * @p: pointer to some position within @str
 *
 * Given a position @p with a UTF-8 encoded string @str, find the start
 * of the previous UTF-8 character starting before @p. Returns %NULL if no
 * UTF-8 characters are present in @str before @p.
 *
 * @p does not have to be at the beginning of a UTF-8 character. No check
 * is made to see if the character found is actually valid other than
 * it starts with an appropriate byte.
 *
 * Returns: (transfer none) (nullable): a pointer to the found character or %NULL.
 */
SysChar *
sys_utf8_find_prev_char (const SysChar *str,
                       const SysChar *p)
{
  while (p > str)
    {
      --p;
      if ((*p & 0xc0) != 0x80)
        return (SysChar *)p;
    }
  return NULL;
}

/**
 * sys_utf8_find_next_char:
 * @p: a pointer to a position within a UTF-8 encoded string
 * @end: (nullable): a pointer to the byte following the end of the string,
 *     or %NULL to indicate that the string is nul-terminated
 *
 * Finds the start of the next UTF-8 character in the string after @p.
 *
 * @p does not have to be at the beginning of a UTF-8 character. No check
 * is made to see if the character found is actually valid other than
 * it starts with an appropriate byte.
 *
 * If @end is %NULL, the return value will never be %NULL: if the end of the
 * string is reached, a pointer to the terminating nul byte is returned. If
 * @end is non-%NULL, the return value will be %NULL if the end of the string
 * is reached.
 *
 * Returns: (transfer none) (nullable): a pointer to the found character or %NULL if @end is
 *    set and is reached
 */
SysChar *
sys_utf8_find_next_char (const SysChar *p,
                       const SysChar *end)
{
  if (end)
    {
      for (++p; p < end && (*p & 0xc0) == 0x80; ++p)
        ;
      return (p >= end) ? NULL : (SysChar *)p;
    }
  else
    {
      for (++p; (*p & 0xc0) == 0x80; ++p)
        ;
      return (SysChar *)p;
    }
}

/**
 * sys_utf8_prev_char:
 * @p: a pointer to a position within a UTF-8 encoded string
 *
 * Finds the previous UTF-8 character in the string before @p.
 *
 * @p does not have to be at the beginning of a UTF-8 character. No check
 * is made to see if the character found is actually valid other than
 * it starts with an appropriate byte. If @p might be the first
 * character of the string, you must use sys_utf8_find_prev_char() instead.
 *
 * Returns: (transfer none) (not nullable): a pointer to the found character
 */
SysChar *
sys_utf8_prev_char (const SysChar *p)
{
  while (true)
    {
      p--;
      if ((*p & 0xc0) != 0x80)
        return (SysChar *)p;
    }
}

/**
 * sys_utf8_strlen:
 * @p: pointer to the start of a UTF-8 encoded string
 * @max: the maximum number of bytes to examine. If @max
 *       is less than 0, then the string is assumed to be
 *       nul-terminated. If @max is 0, @p will not be examined and
 *       may be %NULL. If @max is greater than 0, up to @max
 *       bytes are examined
 *
 * Computes the length of the string in characters, not including
 * the terminating nul character. If the @max'th byte falls in the
 * middle of a character, the last (partial) character is not counted.
 *
 * Returns: the length of the string in characters
 */
SysLong sys_utf8_strlen (const SysChar *p,
               SysSize       max)
{
  SysLong len = 0;
  const SysChar *start = p;
  sys_return_val_if_fail (p != NULL || max == 0, 0);

  if (max < 0)
    {
      while (*p)
        {
          p = sys_utf8_next_char (p);
          ++len;
        }
    }
  else
    {
      if (max == 0 || !*p)
        return 0;

      p = sys_utf8_next_char (p);

      while (p - start < max && *p)
        {
          ++len;
          p = sys_utf8_next_char (p);
        }

      /* only do the last len increment if we got a complete
       * char (don't count partial chars)
       */
      if (p - start <= max)
        ++len;
    }

  return len;
}

/**
 * sys_utf8_substring:
 * @str: a UTF-8 encoded string
 * @start_pos: a character offset within @str
 * @end_pos: another character offset within @str,
 *   or `-1` to indicate the end of the string
 *
 * Copies a substring out of a UTF-8 encoded string.
 * The substring will contain @end_pos - @start_pos characters.
 *
 * Since GLib 2.72, `-1` can be passed to @end_pos to indicate the
 * end of the string.
 *
 * Returns: (transfer full): a newly allocated copy of the requested
 *     substring. Free with sys_free() when no longer needed.
 *
 * Since: 2.30
 */
SysChar *
sys_utf8_substring (const SysChar *str,
                  SysLong        start_pos,
                  SysLong        end_pos)
{
  SysChar *start, *end, *out;

  sys_return_val_if_fail (end_pos >= start_pos || end_pos == -1, NULL);

  start = sys_utf8_offset_to_pointer (str, start_pos);

  if (end_pos == -1)
    {
      SysLong length = sys_utf8_strlen (start, -1);
      end = sys_utf8_offset_to_pointer (start, length);
    }
  else
    {
      end = sys_utf8_offset_to_pointer (start, end_pos - start_pos);
    }

  out = sys_malloc (end - start + 1);
  memcpy (out, start, end - start);
  out[end - start] = 0;

  return out;
}

/**
 * sys_utf8_get_char:
 * @p: a pointer to Unicode character encoded as UTF-8
 *
 * Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
 *
 * If @p does not point to a valid UTF-8 encoded character, results
 * are undefined. If you are not sure that the bytes are complete
 * valid Unicode characters, you should use sys_utf8_get_char_validated()
 * instead.
 *
 * Returns: the resulting character
 */
SysUniChar sys_utf8_get_char (const SysChar *p) {
  int i, mask = 0, len;
  SysUniChar result;
  unsigned char c = (unsigned char) *p;

  UTF8_COMPUTE (c, mask, len);
  if (len == -1)
    return (SysUniChar)-1;
  UTF8_GET (result, p, i, mask, len);

  return result;
}

/**
 * sys_utf8_offset_to_pointer:
 * @str: a UTF-8 encoded string
 * @offset: a character offset within @str
 *
 * Converts from an integer character offset to a pointer to a position
 * within the string.
 *
 * Since 2.10, this function allows to pass a negative @offset to
 * step backwards. It is usually worth stepping backwards from the end
 * instead of forwards if @offset is in the last fourth of the string,
 * since moving forward is about 3 times faster than moving backward.
 *
 * Note that this function doesn't abort when reaching the end of @str.
 * Therefore you should be sure that @offset is within string boundaries
 * before calling that function. Call sys_utf8_strlen() when unsure.
 * This limitation exists as this function is called frequently during
 * text rendering and therefore has to be as fast as possible.
 *
 * Returns: (transfer none): the resulting pointer
 */
SysChar *
sys_utf8_offset_to_pointer  (const SysChar *str,
                           SysLong        offset)
{
  const SysChar *s = str;

  if (offset > 0)
    while (offset--)
      s = sys_utf8_next_char (s);
  else
    {
      const char *s1;

      /* This nice technique for fast backwards stepping
       * through a UTF-8 string was dubbed "stutter stepping"
       * by its inventor, Larry Ewing.
       */
      while (offset)
        {
          s1 = s;
          s += offset;
          while ((*s & 0xc0) == 0x80)
            s--;

          offset += sys_utf8_pointer_to_offset (s, s1);
        }
    }

  return (SysChar *)s;
}

/**
 * sys_utf8_pointer_to_offset:
 * @str: a UTF-8 encoded string
 * @pos: a pointer to a position within @str
 *
 * Converts from a pointer to position within a string to an integer
 * character offset.
 *
 * Since 2.10, this function allows @pos to be before @str, and returns
 * a negative offset in this case.
 *
 * Returns: the resulting character offset
 */
SysLong
sys_utf8_pointer_to_offset (const SysChar *str,
                          const SysChar *pos)
{
  const SysChar *s = str;
  SysLong offset = 0;

  if (pos < str)
    offset = - sys_utf8_pointer_to_offset (pos, str);
  else
    while (s < pos)
      {
        s = sys_utf8_next_char (s);
        offset++;
      }

  return offset;
}


/**
 * sys_utf8_strncpy:
 * @dest: (transfer none): buffer to fill with characters from @src
 * @src: UTF-8 encoded string
 * @n: character count
 *
 * Like the standard C strncpy() function, but copies a given number
 * of characters instead of a given number of bytes. The @src string
 * must be valid UTF-8 encoded text. (Use sys_utf8_validate() on all
 * text before trying to use UTF-8 utility functions with it.)
 *
 * Note you must ensure @dest is at least 4 * @n + 1 to fit the
 * largest possible UTF-8 characters
 *
 * Returns: (transfer none): @dest
 */
SysChar *
sys_utf8_strncpy (SysChar       *dest,
                const SysChar *src,
                SysSize        n)
{
  const SysChar *s = src;
  while (n && *s)
    {
      s = sys_utf8_next_char(s);
      n--;
    }
  strncpy(dest, src, s - src);
  dest[s - src] = 0;
  return dest;
}

/* unicode_strchr */

/**
 * sys_unichar_to_utf8:
 * @c: a Unicode character code
 * @outbuf: (out caller-allocates) (optional): output buffer, must have at
 *       least 6 bytes of space. If %NULL, the length will be computed and
 *       returned and nothing will be written to @outbuf.
 *
 * Converts a single character to UTF-8.
 *
 * Returns: number of bytes written
 */
int
sys_unichar_to_utf8 (SysUniChar c,
                   SysChar   *outbuf)
{
  /* If this gets modified, also update the copy in sys_string_insert_unichar() */
  SysUInt len = 0;
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
    {
      for (i = len - 1; i > 0; --i)
        {
          outbuf[i] = (c & 0x3f) | 0x80;
          c >>= 6;
        }
      outbuf[0] = c | first;
    }

  return len;
}

/**
 * sys_utf8_strchr:
 * @p: a nul-terminated UTF-8 encoded string
 * @len: the maximum length of @p
 * @c: a Unicode character
 *
 * Finds the leftmost occurrence of the given Unicode character
 * in a UTF-8 encoded string, while limiting the search to @len bytes.
 * If @len is -1, allow unbounded search.
 *
 * Returns: (transfer none) (nullable): %NULL if the string does not contain the character,
 *     otherwise, a pointer to the start of the leftmost occurrence
 *     of the character in the string.
 */
SysChar *
sys_utf8_strchr (const char *p,
               SysSize      len,
               SysUniChar    c)
{
  SysChar ch[10];

  SysInt charlen = sys_unichar_to_utf8 (c, ch);
  ch[charlen] = '\0';

  return sys_strstr_len (p, len, ch);
}


/**
 * sys_utf8_strrchr:
 * @p: a nul-terminated UTF-8 encoded string
 * @len: the maximum length of @p
 * @c: a Unicode character
 *
 * Find the rightmost occurrence of the given Unicode character
 * in a UTF-8 encoded string, while limiting the search to @len bytes.
 * If @len is -1, allow unbounded search.
 *
 * Returns: (transfer none) (nullable): %NULL if the string does not contain the character,
 *     otherwise, a pointer to the start of the rightmost occurrence
 *     of the character in the string.
 */
SysChar *
sys_utf8_strrchr (const char *p,
                SysSize      len,
                SysUniChar    c)
{
  SysChar ch[10];

  SysInt charlen = sys_unichar_to_utf8 (c, ch);
  ch[charlen] = '\0';

  return sys_strrstr_len (p, len, ch);
}


/* Like sys_utf8_get_char, but take a maximum length
 * and return (SysUniChar)-2 on incomplete trailing character;
 * also check for malformed or overlong sequences
 * and return (SysUniChar)-1 in this case.
 */
static inline SysUniChar
sys_utf8_get_char_extended (const  SysChar *p,
                          SysSize max_len)
{
  SysSize i, len;
  SysUniChar min_code;
  SysUniChar wc = (SysUChar) *p;
  const SysUniChar partial_sequence = (SysUniChar) -2;
  const SysUniChar malformed_sequence = (SysUniChar) -1;

  if (wc < 0x80)
    {
      return wc;
    }
  else if (SYS_UNLIKELY (wc < 0xc0))
    {
      return malformed_sequence;
    }
  else if (wc < 0xe0)
    {
      len = 2;
      wc &= 0x1f;
      min_code = 1 << 7;
    }
  else if (wc < 0xf0)
    {
      len = 3;
      wc &= 0x0f;
      min_code = 1 << 11;
    }
  else if (wc < 0xf8)
    {
      len = 4;
      wc &= 0x07;
      min_code = 1 << 16;
    }
  else if (wc < 0xfc)
    {
      len = 5;
      wc &= 0x03;
      min_code = 1 << 21;
    }
  else if (wc < 0xfe)
    {
      len = 6;
      wc &= 0x01;
      min_code = 1 << 26;
    }
  else
    {
      return malformed_sequence;
    }

  if (SYS_UNLIKELY (max_len >= 0 && len > (SysSize) max_len))
    {
      for (i = 1; i < (SysSize) max_len; i++)
        {
          if ((((SysUChar *)p)[i] & 0xc0) != 0x80)
            return malformed_sequence;
        }
      return partial_sequence;
    }

  for (i = 1; i < len; ++i)
    {
      SysUniChar ch = ((SysUChar *)p)[i];

      if (SYS_UNLIKELY ((ch & 0xc0) != 0x80))
        {
          if (ch)
            return malformed_sequence;
          else
            return partial_sequence;
        }

      wc <<= 6;
      wc |= (ch & 0x3f);
    }

  if (SYS_UNLIKELY (wc < min_code))
    return malformed_sequence;

  return wc;
}

/**
 * sys_utf8_get_char_validated:
 * @p: a pointer to Unicode character encoded as UTF-8
 * @max_len: the maximum number of bytes to read, or -1 if @p is nul-terminated
 *
 * Convert a sequence of bytes encoded as UTF-8 to a Unicode character.
 * This function checks for incomplete characters, for invalid characters
 * such as characters that are out of the range of Unicode, and for
 * overlong encodings of valid characters.
 *
 * Note that sys_utf8_get_char_validated() returns (SysUniChar)-2 if
 * @max_len is positive and any of the bytes in the first UTF-8 character
 * sequence are nul.
 *
 * Returns: the resulting character. If @p points to a partial
 *     sequence at the end of a string that could begin a valid
 *     character (or if @max_len is zero), returns (SysUniChar)-2;
 *     otherwise, if @p does not point to a valid UTF-8 encoded
 *     Unicode character, returns (SysUniChar)-1.
 */
SysUniChar
sys_utf8_get_char_validated (const SysChar *p,
                           SysSize       max_len)
{
  SysUniChar result;

  if (max_len == 0)
    return (SysUniChar)-2;

  result = sys_utf8_get_char_extended (p, max_len);

  /* Disallow codepoint U+0000 as it’s a nul byte,
   * and all string handling in GLib is nul-terminated */
  if (result == 0 && max_len > 0)
    return (SysUniChar) -2;

  if (result & 0x80000000)
    return result;
  else if (!UNICODE_VALID (result))
    return (SysUniChar)-1;
  else
    return result;
}

#define CONT_BYTE_FAST(p) ((SysUChar)*p++ & 0x3f)

/**
 * sys_utf8_to_ucs4_fast:
 * @str: a UTF-8 encoded string
 * @len: the maximum length of @str to use, in bytes. If @len < 0,
 *     then the string is nul-terminated.
 * @items_written: (out) (optional): location to store the
 *     number of characters in the result, or %NULL.
 *
 * Convert a string from UTF-8 to a 32-bit fixed width
 * representation as UCS-4, assuming valid UTF-8 input.
 * This function is roughly twice as fast as sys_utf8_to_ucs4()
 * but does no error checking on the input. A trailing 0 character
 * will be added to the string after the converted text.
 *
 * Returns: (transfer full): a pointer to a newly allocated UCS-4 string.
 *     This value must be freed with sys_free().
 */
SysUniChar *
sys_utf8_to_ucs4_fast (const SysChar *str,
                     SysLong        len,
                     SysLong       *items_written)
{
  SysUniChar *result;
  SysInt n_chars, i;
  const SysChar *p;

  sys_return_val_if_fail (str != NULL, NULL);

  p = str;
  n_chars = 0;
  if (len < 0)
    {
      while (*p)
        {
          p = sys_utf8_next_char (p);
          ++n_chars;
        }
    }
  else
    {
      while (p < str + len && *p)
        {
          p = sys_utf8_next_char (p);
          ++n_chars;
        }
    }

  result = sys_new (SysUniChar, n_chars + 1);

  p = str;
  for (i=0; i < n_chars; i++)
    {
      SysUChar first = (SysUChar)*p++;
      SysUniChar wc;

      if (first < 0xc0)
        {
          /* We really hope first < 0x80, but we don't want to test an
           * extra branch for invalid input, which this function
           * does not care about. Handling unexpected continuation bytes
           * here will do the least damage. */
          wc = first;
        }
      else
        {
          SysUniChar c1 = CONT_BYTE_FAST(p);
          if (first < 0xe0)
            {
              wc = ((first & 0x1f) << 6) | c1;
            }
          else
            {
              SysUniChar c2 = CONT_BYTE_FAST(p);
              if (first < 0xf0)
                {
                  wc = ((first & 0x0f) << 12) | (c1 << 6) | c2;
                }
              else
                {
                  SysUniChar c3 = CONT_BYTE_FAST(p);
                  wc = ((first & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
                  if (SYS_UNLIKELY (first >= 0xf8))
                    {
                      /* This can't be valid UTF-8, but sys_utf8_next_char()
                       * and company allow out-of-range sequences */
                      SysUniChar mask = 1 << 20;
                      while ((wc & mask) != 0)
                        {
                          wc <<= 6;
                          wc |= CONT_BYTE_FAST(p);
                          mask <<= 5;
                        }
                      wc &= mask - 1;
                    }
                }
            }
        }
      result[i] = wc;
    }
  result[i] = 0;

  if (items_written)
    *items_written = i;

  return result;
}

static SysPointer try_malloc_n (SysSize n_blocks,
    SysSize n_block_bytes,
    SysError **error) {

  SysPointer ptr = sys_try_malloc_n (n_blocks, n_block_bytes);
  if (ptr == NULL)
    sys_error_set_N(error, "%s",
        SYS_("Failed to allocate memory"));
  return ptr;
}

/**
 * sys_utf8_to_ucs4:
 * @str: a UTF-8 encoded string
 * @len: the maximum length of @str to use, in bytes. If @len < 0,
 *     then the string is nul-terminated.
 * @items_read: (out) (optional): location to store number of
  *    bytes read, or %NULL.
 *     If %NULL, then %G_CONVERT_ERROR_PARTIAL_INPUT will be
 *     returned in case @str contains a trailing partial
 *     character. If an error occurs then the index of the
 *     invalid input is stored here.
 * @items_written: (out) (optional): location to store number
 *     of characters written or %NULL. The value here stored does not include
 *     the trailing 0 character.
 * @error: location to store the error occurring, or %NULL to ignore
 *     errors. Any of the errors in #SysConvertError other than
 *     %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from UTF-8 to a 32-bit fixed width
 * representation as UCS-4. A trailing 0 character will be added to the
 * string after the converted text.
 *
 * Returns: (transfer full): a pointer to a newly allocated UCS-4 string.
 *     This value must be freed with sys_free(). If an error occurs,
 *     %NULL will be returned and @error set.
 */
SysUniChar *
sys_utf8_to_ucs4 (const SysChar *str,
                SysLong        len,
                SysLong       *items_read,
                SysLong       *items_written,
                SysError     **error)
{
  SysUniChar *result = NULL;
  SysInt n_chars, i;
  const SysChar *in;

  in = str;
  n_chars = 0;
  while ((len < 0 || str + len - in > 0) && *in)
  {
    SysUniChar wc = sys_utf8_get_char_extended (in, len < 0 ? 6 : str + len - in);
    if (wc & 0x80000000) {

      if (wc == (SysUniChar)-2) {

        if (items_read) {
          break;

        } else {

          sys_error_set_N (error, "%s",
              SYS_("Partial character sequence at end of input"));
        }

      } else {

        sys_error_set_N (error, "%s",
            SYS_("Invalid byte sequence in conversion input"));
      }

      goto err_out;
    }

    n_chars++;

    in = sys_utf8_next_char (in);
  }

  result = try_malloc_n (n_chars + 1, sizeof (SysUniChar), error);
  if (result == NULL)
    goto err_out;

  in = str;
  for (i=0; i < n_chars; i++)
  {
    result[i] = sys_utf8_get_char (in);
    in = sys_utf8_next_char (in);
  }
  result[i] = 0;

  if (items_written)
    *items_written = n_chars;

err_out:
  if (items_read)
    *items_read = in - str;

  return result;
}

/**
 * sys_ucs4_to_utf8:
 * @str: (array length=len) (element-type SysUniChar): a UCS-4 encoded string
 * @len: the maximum length (number of characters) of @str to use.
 *     If @len < 0, then the string is nul-terminated.
 * @items_read: (out) (optional): location to store number of
 *     characters read, or %NULL.
 * @items_written: (out) (optional): location to store number
 *     of bytes written or %NULL. The value here stored does not include the
 *     trailing 0 byte.
 * @error: location to store the error occurring, or %NULL to ignore
 *         errors. Any of the errors in #SysConvertError other than
 *         %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from a 32-bit fixed width representation as UCS-4.
 * to UTF-8. The result will be terminated with a 0 byte.
 *
 * Returns: (transfer full): a pointer to a newly allocated UTF-8 string.
 *     This value must be freed with sys_free(). If an error occurs,
 *     %NULL will be returned and @error set. In that case, @items_read
 *     will be set to the position of the first invalid input character.
 */
  SysChar *
           sys_ucs4_to_utf8 (const SysUniChar *str,
               SysLong           len,
               SysLong          *items_read,
               SysLong          *items_written,
               SysError        **error)
{
  SysInt result_length;
  SysChar *result = NULL;
  SysChar *p;
  SysInt i;

  result_length = 0;
  for (i = 0; len < 0 || i < len ; i++)
  {
    if (!str[i])
      break;

    if (str[i] >= 0x80000000)
    {
      sys_error_set_N (error, "%s",
          SYS_("Character out of range for UTF-8"));
      goto err_out;
    }

    result_length += UTF8_LENGTH (str[i]);
  }

  result = try_malloc_n (result_length + 1, 1, error);
  if (result == NULL)
    goto err_out;

  p = result;

  i = 0;
  while (p < result + result_length)
    p += sys_unichar_to_utf8 (str[i++], p);

  *p = '\0';

  if (items_written)
    *items_written = p - result;

err_out:
  if (items_read)
    *items_read = i;

  return result;
}

#define SURROGATE_VALUE(h,l) (((h) - 0xd800) * 0x400 + (l) - 0xdc00 + 0x10000)

/**
 * sys_utf16_to_utf8:
 * @str: (array length=len) (element-type SysUInt16): a UTF-16 encoded string
 * @len: the maximum length (number of #SysUniChar2) of @str to use.
 *     If @len < 0, then the string is nul-terminated.
 * @items_read: (out) (optional): location to store number of
 *     words read, or %NULL. If %NULL, then %G_CONVERT_ERROR_PARTIAL_INPUT will
 *     be returned in case @str contains a trailing partial character. If
 *     an error occurs then the index of the invalid input is stored here.
 *     It’s guaranteed to be non-negative.
 * @items_written: (out) (optional): location to store number
 *     of bytes written, or %NULL. The value stored here does not include the
 *     trailing 0 byte. It’s guaranteed to be non-negative.
 * @error: location to store the error occurring, or %NULL to ignore
 *     errors. Any of the errors in #SysConvertError other than
 *     %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from UTF-16 to UTF-8. The result will be
 * terminated with a 0 byte.
 *
 * Note that the input is expected to be already in native endianness,
 * an initial byte-order-mark character is not handled specially.
 * sys_convert() can be used to convert a byte buffer of UTF-16 data of
 * ambiguous endianness.
 *
 * Further note that this function does not validate the result
 * string; it may e.g. include embedded NUL characters. The only
 * validation done by this function is to ensure that the input can
 * be correctly interpreted as UTF-16, i.e. it doesn't contain
 * unpaired surrogates or partial character sequences.
 *
 * Returns: (transfer full): a pointer to a newly allocated UTF-8 string.
 *     This value must be freed with sys_free(). If an error occurs,
 *     %NULL will be returned and @error set.
 **/
  SysChar * sys_utf16_to_utf8 (const SysUniChar2  *str,
               SysLong             len,
               SysLong            *items_read,
               SysLong            *items_written,
               SysError          **error)
{
  /* This function and sys_utf16_to_ucs4 are almost exactly identical -
   * The lines that differ are marked.
   */
  const SysUniChar2 *in;
  SysChar *out;
  SysChar *result = NULL;
  SysInt n_bytes;
  SysUniChar high_surrogate;

  sys_return_val_if_fail (str != NULL, NULL);

  n_bytes = 0;
  in = str;
  high_surrogate = 0;
  while ((len < 0 || in - str < len) && *in)
  {
    SysUniChar2 c = *in;
    SysUniChar wc;

    if (c >= 0xdc00 && c < 0xe000) /* low surrogate */
    {
      if (high_surrogate)
      {
        wc = SURROGATE_VALUE (high_surrogate, c);
        high_surrogate = 0;
      }
      else
      {
        sys_error_set_N (error, "%s",
            SYS_("Invalid sequence in conversion input"));
        goto err_out;
      }
    }
    else
    {
      if (high_surrogate)
      {
        sys_error_set_N (error, "%s",
            SYS_("Invalid sequence in conversion input"));
        goto err_out;
      }

      if (c >= 0xd800 && c < 0xdc00) /* high surrogate */
      {
        high_surrogate = c;
        goto next1;
      }
      else
        wc = c;
    }

    /********** DIFFERENT for UTF8/UCS4 **********/
    n_bytes += UTF8_LENGTH (wc);

next1:
    in++;
  }

  if (high_surrogate && !items_read)
  {
    sys_error_set_N (error, "%s",
        SYS_("Partial character sequence at end of input"));
    goto err_out;
  }

  /* At this point, everything is valid, and we just need to convert
  */
  /********** DIFFERENT for UTF8/UCS4 **********/
  result = try_malloc_n (n_bytes + 1, 1, error);
  if (result == NULL)
    goto err_out;

  high_surrogate = 0;
  out = result;
  in = str;
  while (out < result + n_bytes)
  {
    SysUniChar2 c = *in;
    SysUniChar wc;

    if (c >= 0xdc00 && c < 0xe000) /* low surrogate */
    {
      wc = SURROGATE_VALUE (high_surrogate, c);
      high_surrogate = 0;
    }
    else if (c >= 0xd800 && c < 0xdc00) /* high surrogate */
    {
      high_surrogate = c;
      goto next2;
    }
    else
      wc = c;

    /********** DIFFERENT for UTF8/UCS4 **********/
    out += sys_unichar_to_utf8 (wc, out);

next2:
    in++;
  }

  /********** DIFFERENT for UTF8/UCS4 **********/
  *out = '\0';

  if (items_written)
    /********** DIFFERENT for UTF8/UCS4 **********/
    *items_written = out - result;

err_out:
  if (items_read)
    *items_read = in - str;

  return result;
}

/**
 * sys_utf16_to_ucs4:
 * @str: (array length=len) (element-type SysUInt16): a UTF-16 encoded string
 * @len: the maximum length (number of #SysUniChar2) of @str to use.
 *     If @len < 0, then the string is nul-terminated.
 * @items_read: (out) (optional): location to store number of
 *     words read, or %NULL. If %NULL, then %G_CONVERT_ERROR_PARTIAL_INPUT will
 *     be returned in case @str contains a trailing partial character. If
 *     an error occurs then the index of the invalid input is stored here.
 * @items_written: (out) (optional): location to store number
 *     of characters written, or %NULL. The value stored here does not include
 *     the trailing 0 character.
 * @error: location to store the error occurring, or %NULL to ignore
 *     errors. Any of the errors in #SysConvertError other than
 *     %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from UTF-16 to UCS-4. The result will be
 * nul-terminated.
 *
 * Returns: (transfer full): a pointer to a newly allocated UCS-4 string.
 *     This value must be freed with sys_free(). If an error occurs,
 *     %NULL will be returned and @error set.
 */
  SysUniChar *
              sys_utf16_to_ucs4 (const SysUniChar2  *str,
                  SysLong             len,
                  SysLong            *items_read,
                  SysLong            *items_written,
                  SysError          **error)
{
  const SysUniChar2 *in;
  SysChar *out;
  SysChar *result = NULL;
  SysInt n_bytes;
  SysUniChar high_surrogate;

  sys_return_val_if_fail (str != NULL, NULL);

  n_bytes = 0;
  in = str;
  high_surrogate = 0;
  while ((len < 0 || in - str < len) && *in)
  {
    SysUniChar2 c = *in;

    if (c >= 0xdc00 && c < 0xe000) /* low surrogate */
    {
      if (high_surrogate)
      {
        high_surrogate = 0;
      }
      else
      {
        sys_error_set_N (error, "%s",
            SYS_("Invalid sequence in conversion input"));
        goto err_out;
      }
    }
    else
    {
      if (high_surrogate)
      {
        sys_error_set_N (error, "%s",
            SYS_("Invalid sequence in conversion input"));
        goto err_out;
      }

      if (c >= 0xd800 && c < 0xdc00) /* high surrogate */
      {
        high_surrogate = c;
        goto next1;
      }
    }

    /********** DIFFERENT for UTF8/UCS4 **********/
    n_bytes += sizeof (SysUniChar);

next1:
    in++;
  }

  if (high_surrogate && !items_read)
  {
    sys_error_set_N (error, "%s",
        SYS_("Partial character sequence at end of input"));
    goto err_out;
  }

  /* At this point, everything is valid, and we just need to convert
  */
  /********** DIFFERENT for UTF8/UCS4 **********/
  result = try_malloc_n (n_bytes + 4, 1, error);
  if (result == NULL)
    goto err_out;

  high_surrogate = 0;
  out = result;
  in = str;
  while (out < result + n_bytes)
  {
    SysUniChar2 c = *in;
    SysUniChar wc;

    if (c >= 0xdc00 && c < 0xe000) /* low surrogate */
    {
      wc = SURROGATE_VALUE (high_surrogate, c);
      high_surrogate = 0;
    }
    else if (c >= 0xd800 && c < 0xdc00) /* high surrogate */
    {
      high_surrogate = c;
      goto next2;
    }
    else
      wc = c;

    /********** DIFFERENT for UTF8/UCS4 **********/
    *(SysUniChar *)out = wc;
    out += sizeof (SysUniChar);

next2:
    in++;
  }

  /********** DIFFERENT for UTF8/UCS4 **********/
  *(SysUniChar *)out = 0;

  if (items_written)
    /********** DIFFERENT for UTF8/UCS4 **********/
    *items_written = (out - result) / sizeof (SysUniChar);

err_out:
  if (items_read)
    *items_read = in - str;

  return (SysUniChar *)result;
}

/**
 * sys_utf8_to_utf16:
 * @str: a UTF-8 encoded string
 * @len: the maximum length (number of bytes) of @str to use.
 *     If @len < 0, then the string is nul-terminated.
 * @items_read: (out) (optional): location to store number of
 *     bytes read, or %NULL. If %NULL, then %G_CONVERT_ERROR_PARTIAL_INPUT will
 *     be returned in case @str contains a trailing partial character. If
 *     an error occurs then the index of the invalid input is stored here.
 * @items_written: (out) (optional): location to store number
 *     of #SysUniChar2 written, or %NULL. The value stored here does not include
 *     the trailing 0.
 * @error: location to store the error occurring, or %NULL to ignore
 *     errors. Any of the errors in #SysConvertError other than
 *     %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from UTF-8 to UTF-16. A 0 character will be
 * added to the result after the converted text.
 *
 * Returns: (transfer full): a pointer to a newly allocated UTF-16 string.
 *     This value must be freed with sys_free(). If an error occurs,
 *     %NULL will be returned and @error set.
 */
  SysUniChar2 *
               sys_utf8_to_utf16 (const SysChar *str,
                   SysLong        len,
                   SysLong       *items_read,
                   SysLong       *items_written,
                   SysError     **error)
{
  SysUniChar2 *result = NULL;
  SysInt n16;
  const SysChar *in;
  SysInt i;

  sys_return_val_if_fail (str != NULL, NULL);

  in = str;
  n16 = 0;
  while ((len < 0 || str + len - in > 0) && *in)
  {
    SysUniChar wc = sys_utf8_get_char_extended (in, len < 0 ? 6 : str + len - in);
    if (wc & 0x80000000)
    {
      if (wc == (SysUniChar)-2)
      {
        if (items_read)
          break;
        else
          sys_error_set_N (error, "%s",
              SYS_("Partial character sequence at end of input"));
      }
      else
        sys_error_set_N (error, "%s",
            SYS_("Invalid byte sequence in conversion input"));

      goto err_out;
    }

    if (wc < 0xd800)
      n16 += 1;
    else if (wc < 0xe000)
    {
      sys_error_set_N (error, "%s",
          SYS_("Invalid sequence in conversion input"));

      goto err_out;
    }
    else if (wc < 0x10000)
      n16 += 1;
    else if (wc < 0x110000)
      n16 += 2;
    else
    {
      sys_error_set_N (error, "%s",
          SYS_("Character out of range for UTF-16"));

      goto err_out;
    }

    in = sys_utf8_next_char (in);
  }

  result = try_malloc_n (n16 + 1, sizeof (SysUniChar2), error);
  if (result == NULL)
    goto err_out;

  in = str;
  for (i = 0; i < n16;)
  {
    SysUniChar wc = sys_utf8_get_char (in);

    if (wc < 0x10000)
    {
      result[i++] = wc;
    }
    else
    {
      result[i++] = (wc - 0x10000) / 0x400 + 0xd800;
      result[i++] = (wc - 0x10000) % 0x400 + 0xdc00;
    }

    in = sys_utf8_next_char (in);
  }

  result[i] = 0;

  if (items_written)
    *items_written = n16;

err_out:
  if (items_read)
    *items_read = in - str;

  return result;
}

/**
 * sys_ucs4_to_utf16:
 * @str: (array length=len) (element-type SysUniChar): a UCS-4 encoded string
 * @len: the maximum length (number of characters) of @str to use.
 *     If @len < 0, then the string is nul-terminated.
 * @items_read: (out) (optional): location to store number of
 *     bytes read, or %NULL. If an error occurs then the index of the invalid
 *     input is stored here.
 * @items_written: (out) (optional): location to store number
 *     of #SysUniChar2  written, or %NULL. The value stored here does not include
 *     the trailing 0.
 * @error: location to store the error occurring, or %NULL to ignore
 *     errors. Any of the errors in #SysConvertError other than
 *     %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from UCS-4 to UTF-16. A 0 character will be
 * added to the result after the converted text.
 *
 * Returns: (transfer full): a pointer to a newly allocated UTF-16 string.
 *     This value must be freed with sys_free(). If an error occurs,
 *     %NULL will be returned and @error set.
 */
  SysUniChar2 *
               sys_ucs4_to_utf16 (const SysUniChar  *str,
                   SysLong            len,
                   SysLong           *items_read,
                   SysLong           *items_written,
                   SysError         **error)
{
  SysUniChar2 *result = NULL;
  SysInt n16;
  SysInt i, j;

  n16 = 0;
  i = 0;
  while ((len < 0 || i < len) && str[i])
  {
    SysUniChar wc = str[i];

    if (wc < 0xd800)
      n16 += 1;
    else if (wc < 0xe000)
    {
      sys_error_set_N(error, "%s",
          SYS_("Invalid sequence in conversion input"));

      goto err_out;
    }
    else if (wc < 0x10000)
      n16 += 1;
    else if (wc < 0x110000)
      n16 += 2;
    else
    {
      sys_error_set_N (error, "%s",
          SYS_("Character out of range for UTF-16"));

      goto err_out;
    }

    i++;
  }

  result = try_malloc_n (n16 + 1, sizeof (SysUniChar2), error);
  if (result == NULL)
    goto err_out;

  for (i = 0, j = 0; j < n16; i++)
  {
    SysUniChar wc = str[i];

    if (wc < 0x10000)
    {
      result[j++] = wc;
    }
    else
    {
      result[j++] = (wc - 0x10000) / 0x400 + 0xd800;
      result[j++] = (wc - 0x10000) % 0x400 + 0xdc00;
    }
  }
  result[j] = 0;

  if (items_written)
    *items_written = n16;

err_out:
  if (items_read)
    *items_read = i;

  return result;
}

#define VALIDATE_BYTE(mask, expect)                      \
  SYS_STMT_START {                                         \
    if (SYS_UNLIKELY((*(SysUChar *)p & (mask)) != (expect))) \
    goto error;                                        \
  } SYS_STMT_END

/* see IETF RFC 3629 Section 4 */

  static const SysChar *
fast_validate (const char *str)

{
  const SysChar *p;

  for (p = str; *p; p++)
  {
    if (*(SysUChar *)p < 128)
      /* done */;
    else
    {
      const SysChar *last;

      last = p;
      if (*(SysUChar *)p < 0xe0) /* 110xxxxx */
      {
        if (SYS_UNLIKELY (*(SysUChar *)p < 0xc2))
          goto error;
      }
      else
      {
        if (*(SysUChar *)p < 0xf0) /* 1110xxxx */
        {
          switch (*(SysUChar *)p++ & 0x0f)
          {
            case 0:
              VALIDATE_BYTE(0xe0, 0xa0); /* 0xa0 ... 0xbf */
              break;
            case 0x0d:
              VALIDATE_BYTE(0xe0, 0x80); /* 0x80 ... 0x9f */
              break;
            default:
              VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
          }
        }
        else if (*(SysUChar *)p < 0xf5) /* 11110xxx excluding out-of-range */
        {
          switch (*(SysUChar *)p++ & 0x07)
          {
            case 0:
              VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
              if (SYS_UNLIKELY((*(SysUChar *)p & 0x30) == 0))
                goto error;
              break;
            case 4:
              VALIDATE_BYTE(0xf0, 0x80); /* 0x80 ... 0x8f */
              break;
            default:
              VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
          }
          p++;
          VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
        }
        else
          goto error;
      }

      p++;
      VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */

      continue;

error:
      return last;
    }
  }

  return p;
}

  static const SysChar *
fast_validate_len (const char *str,
    SysSize      max_len)

{
  const SysChar *p;

  sys_assert (max_len >= 0);

  for (p = str; ((p - str) < max_len) && *p; p++)
  {
    if (*(SysUChar *)p < 128)
      /* done */;
    else
    {
      const SysChar *last;

      last = p;
      if (*(SysUChar *)p < 0xe0) /* 110xxxxx */
      {
        if (SYS_UNLIKELY (max_len - (p - str) < 2))
          goto error;

        if (SYS_UNLIKELY (*(SysUChar *)p < 0xc2))
          goto error;
      }
      else
      {
        if (*(SysUChar *)p < 0xf0) /* 1110xxxx */
        {
          if (SYS_UNLIKELY (max_len - (p - str) < 3))
            goto error;

          switch (*(SysUChar *)p++ & 0x0f)
          {
            case 0:
              VALIDATE_BYTE(0xe0, 0xa0); /* 0xa0 ... 0xbf */
              break;
            case 0x0d:
              VALIDATE_BYTE(0xe0, 0x80); /* 0x80 ... 0x9f */
              break;
            default:
              VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
          }
        }
        else if (*(SysUChar *)p < 0xf5) /* 11110xxx excluding out-of-range */
        {
          if (SYS_UNLIKELY (max_len - (p - str) < 4))
            goto error;

          switch (*(SysUChar *)p++ & 0x07)
          {
            case 0:
              VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
              if (SYS_UNLIKELY((*(SysUChar *)p & 0x30) == 0))
                goto error;
              break;
            case 4:
              VALIDATE_BYTE(0xf0, 0x80); /* 0x80 ... 0x8f */
              break;
            default:
              VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
          }
          p++;
          VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */
        }
        else
          goto error;
      }

      p++;
      VALIDATE_BYTE(0xc0, 0x80); /* 10xxxxxx */

      continue;

error:
      return last;
    }
  }

  return p;
}

/**
 * sys_utf8_validate:
 * @str: (array length=max_len) (element-type SysUInt8): a pointer to character data
 * @max_len: max bytes to validate, or -1 to go until NUL
 * @end: (out) (optional) (transfer none): return location for end of valid data
 *
 * Validates UTF-8 encoded text. @str is the text to validate;
 * if @str is nul-terminated, then @max_len can be -1, otherwise
 * @max_len should be the number of bytes to validate.
 * If @end is non-%NULL, then the end of the valid range
 * will be stored there (i.e. the start of the first invalid
 * character if some bytes were invalid, or the end of the text
 * being validated otherwise).
 *
 * Note that sys_utf8_validate() returns %false if @max_len is
 * positive and any of the @max_len bytes are nul.
 *
 * Returns %true if all of @str was valid. Many GLib and GTK+
 * routines require valid UTF-8 as input; so data read from a file
 * or the network should be checked with sys_utf8_validate() before
 * doing anything else with it.
 *
 * Returns: %true if the text was valid UTF-8
 */
  SysBool
sys_utf8_validate (const char   *str,
    SysSize        max_len,
    const SysChar **end)

{
  const SysChar *p;

  if (max_len >= 0)
    return sys_utf8_validate_len (str, max_len, end);

  p = fast_validate (str);

  if (end)
    *end = p;

  if (*p != '\0')
    return false;
  else
    return true;
}

/**
 * sys_utf8_validate_len:
 * @str: (array length=max_len) (element-type SysUInt8): a pointer to character data
 * @max_len: max bytes to validate
 * @end: (out) (optional) (transfer none): return location for end of valid data
 *
 * Validates UTF-8 encoded text.
 *
 * As with sys_utf8_validate(), but @max_len must be set, and hence this function
 * will always return %false if any of the bytes of @str are nul.
 *
 * Returns: %true if the text was valid UTF-8
 * Since: 2.60
 */
  SysBool
         sys_utf8_validate_len (const char   *str,
             SysSize         max_len,
             const SysChar **end)

{
  const SysChar *p;

  p = fast_validate_len (str, max_len);

  if (end)
    *end = p;

  if (p != str + max_len)
    return false;
  else
    return true;
}

/**
 * sys_unichar_validate:
 * @ch: a Unicode character
 *
 * Checks whether @ch is a valid Unicode character. Some possible
 * integer values of @ch will not be valid. 0 is considered a valid
 * character, though it's normally a string terminator.
 *
 * Returns: %true if @ch is a valid Unicode character
 **/
  SysBool
sys_unichar_validate (SysUniChar ch)
{
  return UNICODE_VALID (ch);
}

/**
 * sys_utf8_strreverse:
 * @str: a UTF-8 encoded string
 * @len: the maximum length of @str to use, in bytes. If @len < 0,
 *     then the string is nul-terminated.
 *
 * Reverses a UTF-8 string. @str must be valid UTF-8 encoded text.
 * (Use sys_utf8_validate() on all text before trying to use UTF-8
 * utility functions with it.)
 *
 * This function is intended for programmatic uses of reversed strings.
 * It pays no attention to decomposed characters, combining marks, byte
 * order marks, directional indicators (LRM, LRO, etc) and similar
 * characters which might need special handling when reversing a string
 * for display purposes.
 *
 * Note that unlike sys_strreverse(), this function returns
 * newly-allocated memory, which should be freed with sys_free() when
 * no longer needed.
 *
 * Returns: (transfer full): a newly-allocated string which is the reverse of @str
 *
 * Since: 2.2
 */
  SysChar *
           sys_utf8_strreverse (const SysChar *str,
               SysSize       len)
{
  SysChar *r, *result;
  const SysChar *p;

  if (len < 0)
    len = strlen (str);

  result = sys_new (SysChar, len + 1);
  r = result + len;
  p = str;
  while (r > result)
  {
    SysChar *m, skip = sys_utf8_skip[*(SysUChar*) p];
    r -= skip;
    sys_assert (r >= result);
    for (m = r; skip; skip--)
      *m++ = *p++;
  }
  result[len] = 0;

  return result;
}

/**
 * sys_utf8_make_valid:
 * @str: string to coerce into UTF-8
 * @len: the maximum length of @str to use, in bytes. If @len < 0,
 *     then the string is nul-terminated.
 *
 * If the provided string is valid UTF-8, return a copy of it. If not,
 * return a copy in which bytes that could not be interpreted as valid Unicode
 * are replaced with the Unicode replacement character (U+FFFD).
 *
 * For example, this is an appropriate function to use if you have received
 * a string that was incorrectly declared to be UTF-8, and you need a valid
 * UTF-8 version of it that can be logged or displayed to the user, with the
 * assumption that it is close enough to ASCII or UTF-8 to be mostly
 * readable as-is.
 *
 * Returns: (transfer full): a valid UTF-8 string whose content resembles @str
 *
 * Since: 2.52
 */
  SysChar *
           sys_utf8_make_valid (const SysChar *str,
               SysSize       len)
{
  SysString *string;
  const SysChar *remainder, *invalid;
  SysSize remaininsys_bytes, valid_bytes;

  sys_return_val_if_fail (str != NULL, NULL);

  if (len < 0)
    len = strlen (str);

  string = NULL;
  remainder = str;
  remaininsys_bytes = len;

  while (remaininsys_bytes != 0)
  {
    if (sys_utf8_validate (remainder, remaininsys_bytes, &invalid))
      break;
    valid_bytes = invalid - remainder;

    if (string == NULL)
      string = sys_string_sized_new (remaininsys_bytes);

    sys_string_append_len (string, remainder, valid_bytes);
    /* append U+FFFD REPLACEMENT CHARACTER */
    sys_string_append (string, "\357\277\275");

    remaininsys_bytes -= valid_bytes + 1;
    remainder = invalid + 1;
  }

  if (string == NULL)
    return sys_strndup (str, len);

  sys_string_append_len (string, remainder, remaininsys_bytes);
  sys_string_append_c (string, '\0');

  sys_assert (sys_utf8_validate (string->str, -1, NULL));

  return sys_string_free (string, false);
}

/**
 * sys_utf8_casefold:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts a string into a form that is independent of case. The
 * result will not correspond to any particular case, but can be
 * compared for equality or ordered with the results of calling
 * sys_utf8_casefold() on other strings.
 *
 * Note that calling sys_utf8_casefold() followed by sys_utf8_collate() is
 * only an approximation to the correct linguistic case insensitive
 * ordering, though it is a fairly good one. Getting this exactly
 * right would require a more sophisticated collation function that
 * takes case sensitivity into account. GLib does not currently
 * provide such a function.
 *
 * Returns: a newly allocated string, that is a
 *   case independent form of @str.
 **/
  SysChar *
sys_utf8_casefold (const SysChar *str,
    SysSize       len)
{
  SysString *result;
  const char *p;

  sys_return_val_if_fail (str != NULL, NULL);

  result = sys_string_new (NULL);
  p = str;
  while ((len < 0 || p < str + len) && *p)
  {
    SysUniChar ch = sys_utf8_get_char (p);

    int start = 0;
    int end = ARRAY_SIZE (casefold_table);

    if (ch >= casefold_table[start].ch &&
        ch <= casefold_table[end - 1].ch)
    {
      while (true)
      {
        int half = (start + end) / 2;
        if (ch == casefold_table[half].ch)
        {
          sys_string_append (result, casefold_table[half].data);
          goto next;
        }
        else if (half == start)
          break;
        else if (ch > casefold_table[half].ch)
          start = half;
        else
          end = half;
      }
    }

    sys_string_append_unichar (result, sys_unichar_tolower (ch));

next:
    p = sys_utf8_next_char (p);
  }

  return sys_string_free (result, false);
}

/**
 * sys_utf8_strdown:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts all Unicode characters in the string that have a case
 * to lowercase. The exact manner that this is done depends
 * on the current locale, and may result in the number of
 * characters in the string changing.
 *
 * Returns: a newly allocated string, with all characters
 *    converted to lowercase.
 **/
  SysChar *
sys_utf8_strdown (const SysChar *str,
    SysSize       len)
{
  SysSize result_len;
  LocaleType locale_type;
  SysChar *result;

  sys_return_val_if_fail (str != NULL, NULL);

  locale_type = sys_unicode_get_locale_type ();

  /*
   * We use a two pass approach to keep memory management simple
   */
  result_len = sys_unicode_real_tolower (str, len, NULL, locale_type);
  result = sys_malloc (result_len + 1);
  sys_unicode_real_tolower (str, len, result, locale_type);
  result[result_len] = '\0';

  return result;
}

/**
 * sys_utf8_strup:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts all Unicode characters in the string that have a case
 * to uppercase. The exact manner that this is done depends
 * on the current locale, and may result in the number of
 * characters in the string increasing. (For instance, the
 * German ess-zet will be changed to SS.)
 *
 * Returns: a newly allocated string, with all characters
 *    converted to uppercase.
 **/
  SysChar *
sys_utf8_strup (const SysChar *str,
    SysSize       len)
{
  SysSize result_len;
  LocaleType locale_type;
  SysChar *result;

  sys_return_val_if_fail (str != NULL, NULL);

  locale_type = sys_unicode_get_locale_type ();

  /*
   * We use a two pass approach to keep memory management simple
   */
  result_len = sys_unicode_real_toupper (str, len, NULL, locale_type);
  result = sys_malloc (result_len + 1);
  sys_unicode_real_toupper (str, len, result, locale_type);
  result[result_len] = '\0';

  return result;
}
