#include <System/Utils/SysStr.h>
#include <System/Utils/SysError.h>
#include <System/Platform/Common/SysOs.h>
#include <System/Platform/Common/SysMem.h>

#define SYS_BYTE_INIT_SIZE 4096

SysChar* sys_strstr(const SysChar *s, const SysChar* delim) {
  sys_return_val_if_fail(s != NULL, NULL);
  sys_return_val_if_fail(delim != NULL, NULL);

  return strstr(s, delim);
}

SysChar *sys_strchug (SysChar *string)
{
  SysUChar *start;

  sys_return_val_if_fail (string != NULL, NULL);

  for (start = (SysUChar*) string; *start && isspace (*start); start++)
    ;

  memmove (string, start, strlen ((SysChar *) start) + 1);

  return string;
}

SysBool sys_str_trim_end(SysChar *string, const SysChar c) {
  sys_return_val_if_fail (string != NULL, false);

  SysChar * p = strrchr(string, c);
  if(!p) { return false;}
  *p = '\0';

  return true;
}

SysBool sys_str_strip(SysChar *s) {

  return sys_strchomp(sys_strchug(s));
}

SysChar * sys_strchomp (SysChar *string) {
  SysSSize len;

  sys_return_val_if_fail (string != NULL, NULL);

  len = strlen (string);
  while (len--)
  {
    if (isspace ((SysUChar) string[len]))
      string[len] = '\0';
    else
      break;
  }

  return string;
}

/**
 * sys_strsplit: split with delimiter, free it only once
 * @s: dst string
 * @delim: seperate char
 * @count: split count.
 *
 * Returns: new points + string copy, just free returned pointer.
 */
SysChar **sys_strsplit(const SysChar * s, const SysChar *delim, SysInt * count) {
  sys_return_val_if_fail(s != NULL, NULL);
  sys_return_val_if_fail(delim != NULL, NULL);
  sys_return_val_if_fail(*count == 0, NULL);

  SysChar **ptrs;
  SysChar* sp, *nsp;
  SysSSize sp_size;
  SysSSize s_len;
  SysSSize delim_len;
  SysInt delim_count;

  sp = (SysChar *)s;
  s_len = strlen(s);
  delim_len = strlen(delim);
  delim_count = 1;

  while ((sp = strstr(sp, delim)) != NULL) {
    sp += delim_len;
    delim_count++;
  }

  sp_size = sizeof(SysChar *) * (delim_count + 1);

  /* array ptr and str with one malloc  */
  sp = sys_malloc(sp_size + s_len + 1);
  if(!sp) { return NULL; }
  ptrs = (SysChar **)sp;

  nsp = sp + sp_size;
  sys_strcpy(nsp, s);
  *ptrs++ = nsp;

  if(delim_count > 1) {
    while ((nsp = strstr(nsp, delim)) != NULL) {
      *nsp = '\0';
      nsp += delim_len;
      *ptrs++ = nsp;
    }
  }
  *count = delim_count;

  return (SysChar **)sp;

}

/**
 * sys_strcat_M:
 *   copy v2 to v1, return v1, realloc v1 if v1 not enough.
 *   simple method for concat string, use sys_strinsys_* for better api.
 *
 *  example:
 *    SysSSize mlen = 200, len = 0;
 *    SysChar *s = sys_str_newsize(SysChar, mlen);
 *    sys_strmcat(&s, &mlen, &len, "var");
 *    sys_strmcat(&s, &mlen, &len, "=");
 *    sys_strmcat(&s, &mlen, &len, "foo");
 *    sys_free(s);
 *
 * @v1: *v1 is the string to store.
 * @v1_max: *v1_max is max length of v1 string, may be realloc when not enough.
 * @v2: the string to append.
 *
 * Returns: v1 string length.
 */
void sys_strmcat(SysChar** v1, SysSize* v1_max, SysSize* len, const SysChar* v2) {
  sys_return_if_fail(*v1 != NULL);
  sys_return_if_fail(v2 != NULL);
  sys_return_if_fail(v1_max != NULL && "sys_strcat max_len should not be null.");

  SysChar *nstr;
  SysSize nearup;
  SysSize v1len;
  SysSize v2len;
  SysSize nlen;

  v1len = strlen(*v1);
  v2len = strlen(v2);

  nlen = v1len + v2len;

  if (nlen >= *v1_max) {
    nearup = sys_nearest_pow((SysUInt)(nlen + 1));

    nstr = sys_realloc(*v1, nearup);
    if (nstr == NULL) {
      sys_abort_N("%s", "renew failed");
    }

    *v1 = nstr;
    *v1_max = nearup;
  }

  sys_memcpy(*v1 + v1len, *v1_max, v2, v2len);
  *(*v1 + nlen) = '\0';
  *len = nlen;
}

/**
 * sys_strconcat:
 * @string1: the first string to add, which must not be %NULL
 * @...: a %NULL-terminated list of strings to append to the string
 *
 * Concatenates all of the given strings into one long string. The
 * returned string should be freed with sys_free() when no longer needed.
 *
 * The variable argument list must end with %NULL. If you forget the %NULL,
 * sys_strconcat() will start appending random memory junk to your string.
 *
 * Note that this function is usually not the right function to use to
 * assemble a translated message from pieces, since proper translation
 * often requires the pieces to be reordered.
 *
 * Returns: a newly-allocated string containing all the string arguments
 */
SysChar* sys_strconcat (const SysChar *string1, ...) {
  SysSSize   l;
  va_list args;
  SysChar   *s;
  SysChar   *concat;
  SysChar   *ptr;

  if (!string1)
    return NULL;

  l = 1 + strlen (string1);
  va_start (args, string1);
  s = va_arg (args, SysChar*);
  while (s)
  {
    l += strlen (s);
    s = va_arg (args, SysChar*);
  }
  va_end (args);

  concat = sys_new (SysChar, l);
  ptr = concat;

  ptr = sys_strpcpy (ptr, string1);
  va_start (args, string1);
  s = va_arg (args, SysChar*);
  while (s) {
    ptr = sys_strpcpy (ptr, s);
    s = va_arg (args, SysChar*);
  }
  va_end (args);

  return concat;
}

/**
 * sys_strlcat: copy v2 to v1, return v1 if v1 not enough.
 *
 * Returns: return v1
 */
SysChar* sys_strlcat(SysChar* v1, SysSSize v1_max, const SysChar* v2) {
  sys_return_val_if_fail(v1 != NULL, NULL);
  sys_return_val_if_fail(v2 != NULL, NULL);

  SysSSize v1_len = sys_strlen(v1, v1_max);
  SysSSize v2_len = strlen(v2);
  SysSSize nlen = v1_len + v2_len;

  if (nlen > v1_max) {
    sys_warning_N("string has no enough space ? %s", v1);
    return v1;
  }

  sys_memcpy(v1 + v1_len, v1_max, v2, v2_len);
  v1[nlen] = '\0';

  return v1;
}

SysChar *sys_strpncpy(SysChar *dst, SysInt n, const SysChar *src) {
  sys_return_val_if_fail(dst != NULL, NULL);
  sys_return_val_if_fail(src != NULL, NULL);

  const SysChar *s = src;
  SysChar *d = dst;

  while (*s != '\0') {
    n--;
    if (n < 0) {
      break;
    }

    *d++ = *s++;
  }

  return d;
}

SysChar* sys_strpcpy(SysChar* dst, const SysChar* src) {
  sys_return_val_if_fail(dst != NULL, NULL);
  sys_return_val_if_fail(src != NULL, NULL);

  const SysChar *s = src;
  SysChar *d = dst;

  while (*s != '\0') {
    *d++ = *s++;
  }

  return d;
}

/**
 * sys_strdup: duplicate str with '\0' terminated.
 *
 * Returns: new alloc string.
 */
SysChar* sys_strdup(const SysChar *s) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysSSize len = strlen(s);
  SysChar *n = sys_new(SysChar, len + 1);
  memcpy(n, s, len);
  n[len] = '\0';

  return n;
}

/**
 * sys_strndup: copy length of s.
 *
 * Returns: new alloc str.
 */
SysChar* sys_strndup(const SysChar *s, SysSSize len) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysChar *n = sys_new(SysChar, len + 1);
  memcpy(n, s, len);
  n[len] = '\0';

  return n;
}

/**
 * sys_vasprintf: format function
 * use vsnprintf on linux, use _vsprintf_p on windows.
 *
 * Returns: void
 */

/**
 * sys_asprintf: create new format string, *str must NULL.
 * @str: value
 * @format: printf format
 *
 * Returns: length of string
 */
SysInt sys_asprintf(SysChar **str, const SysChar *format, ...) {
  sys_return_val_if_fail(*str == NULL && "sys_asprintf *str should be NULL", -1);

  SysInt len;

  va_list args;
  va_start(args, format);

  len = sys_vasprintf(str, format, args);

  va_end(args);

  return len;
}

/**
 * sys_printf: print format
 * @format:
 *
 * Returns: length of string.
 */
SysInt sys_printf(const SysChar *format, ...) {
  SysInt len;

  va_list args;

  SYS_LEAK_IGNORE_BEGIN;

  va_start(args, format);
  len = sys_vprintf(format, args);
  va_end(args);

  SYS_LEAK_IGNORE_END;

  return len;
}

/**
 * sys_print: use putc to print.
 * @str: print value
 *
 * Returns: length of string.
 */
void sys_print(const SysChar *str) {
  sys_return_if_fail(str != NULL);

  SYS_LEAK_IGNORE_BEGIN;

  fputs(str, stdout);
  fflush(stdout);

  SYS_LEAK_IGNORE_END;
}

/**
 * sys_snprintf: format string to buff.
 *   use vsnprintf on linux, use vsprintf_s on windows.
 *
 * @str: dst string.
 * @size: byte of string.
 * @format: format string.
 *
 * Returns: length of string.
 */
SysInt sys_snprintf(SysChar *str, SysSSize size, const SysChar *format, ...) {
  SysInt len;

  va_list args;
  va_start(args, format);

  len = sys_vsprintf(str, size, format, args);

  va_end(args);

  return len;
}

/**
 * sys_strdup_printf: format dup
 * @format:
 *
 * Returns: new allocted string.
 */
SysChar *sys_strdup_printf(const SysChar *format, ...) {
  SysChar *str = NULL;

  va_list args;
  va_start(args, format);

  sys_vasprintf(&str, format, args);

  va_end(args);

  return str;
}

/**
 * sys_strjoin_array:
 *
 * insert string between string array.
 * get with string length.
 *
 * @delim: the string insert between string array.
 * @arr: src string array.
 * @dstlen: out len for dst string length.
 *
 * Returns: joined string with delimiter.
 */
SysChar* sys_strjoin_array(const SysChar *delim, const SysChar **arr, SysSSize *dstlen) {
  sys_return_val_if_fail(delim != NULL, NULL);
  sys_return_val_if_fail(arr != NULL, NULL);

  SysChar *nstr, *ptr;
  SysSSize len;
  SysSSize dlen = strlen(delim);

  const SysChar **na = arr;
  if (!(*na)) {
    nstr = sys_strdup("");
    return nstr;
  }

  len = strlen((*na));

  while(++(*na)) {
    len += dlen + strlen((*na));
  }

  nstr = sys_new0(SysChar, len + 1);
  na = arr;

  ptr = sys_strpcpy(nstr, (*na));
  while (++(*na)) {
    ptr = sys_strpcpy(ptr, delim);
    ptr = sys_strpcpy(ptr, (*na));
  }

  *dstlen = len;
  return nstr;
}

SysChar *sys_strjoinv(const SysChar *delim, va_list args) {
  SysChar *s, *nstr, *ptr;
  SysSSize len;
  SysSSize dlen;
  va_list nargs;

  dlen = strlen(delim);

  va_copy(nargs, args);
  s = va_arg(nargs, SysChar *);
  len = strlen(s);

  for (s = va_arg(nargs, SysChar *); s;) {
    len += dlen + strlen(s);
    s = va_arg(nargs, SysChar *);
  }

  nstr = sys_new0(SysChar, len + 1);

  s = va_arg(args, SysChar *);
  ptr = sys_strpcpy(nstr, s);
  for (s = va_arg(args, SysChar *); s;) {
    ptr = sys_strpcpy(ptr, delim);
    ptr = sys_strpcpy(ptr, s);

    s = va_arg(args, SysChar *);
  }

  return nstr;
}

SysChar *_sys_strjoin(const SysChar *delim, ...) {
  SysChar *s;

  va_list args;
  va_start(args, delim);

  s = sys_strjoinv(delim, args);

  va_end(args);

  return s;
}

/**
 * sys_bin_str_full: convert for binary str.
 *
 *  use sys_bin_new() for alloc new buffer,
 *  use sys_bin_tostr convet it to string.
 *  example:
 *    SysInt a = 16;
 *    SysChar *buf = sys_bin_new(a, sizeof(a));
 *    buf = sys_bin_tostr(buf, a, sizeof(a));
 *
 * @buffer: data store buffer.
 * @bufsize: 8 * sizeof(type) + 1
 * @sbyte: sizeof(type)
 * @ptr: target pointer
 *
 * Returns: buffer
 */
SysChar* sys_bin_str_full(SysChar *buffer, SysSSize bufsize, SysSSize const sbyte, void const *const ptr) {
  SysUChar *b = (SysUChar *)ptr;
  SysUChar byte;

  if (bufsize != (sbyte * 8 + 1)) {
    perror("sys_bin_tostr bufsize not equal sbyte * 8 + 1.");
    abort();
  }

  SysInt i, j, m = 0;
  for (i = (SysInt)sbyte - 1; i >= 0; i--) {
    for (j = 7; j >= 0; j--) {
      byte = (b[i] >> j) & 1;
      buffer[m++] = byte ? '1' : '0';
    }
  }
  buffer[m] = '\0';

  return buffer;
}

/**
 * sys_strlen: strlen
 * @s: src string.
 * @max: max length of string.
 *
 * Returns: length of string
 */
SysSSize sys_strlen(const SysChar *s, SysSSize max) {
  SysSSize len;

  for (len = 0; len < max; len++, s++) {
    if (!*s) {
      break;
    }
  }
  return len;
}

SysBool sys_str_equal(const SysChar *s1, const SysChar *s2) {
  sys_return_val_if_fail(s1 != NULL, false);
  sys_return_val_if_fail(s2 != NULL, false);

  return sys_strcmp(s1, s2) == 0;
}

SysInt sys_strcmp(const SysChar *s1, const SysChar *s2) {
  if (!s1)
    return -(s1 != s2);
  if (!s2)
    return s1 != s2;

  return strcmp(s1, s2);
}

SysChar *sys_strupper(SysChar *s) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysUChar *ns = (SysUChar *)s;
  for (; *ns; ns++) {
    *ns = (SysUChar)toupper(*ns);
  }

  return s;
}

SysChar *sys_strlower(SysChar *s) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysUChar *ns = (SysUChar *)s;
  for (;*ns; ns++) {
    *ns = (SysUChar)tolower(*ns);
  }

  return s;
}

SysInt sys_strncmp(const SysChar *s1, const SysChar *s2, SysInt max) {
  sys_return_val_if_fail(s1 != NULL, 0);
  sys_return_val_if_fail(s2 != NULL, 0);

  if (!s1)
    return -(s1 != s2);
  if (!s2)
    return s1 != s2;

  return strncmp(s1, s2, max);
}

/**
 * sys_str_override_c: replace all oldchar to newchar in str.
 * @str: source string can be modified.
 * @oldchar: old char.
 * @newchar: new char.
 *
 * Returns: void
 */
SysBool sys_str_override_c(SysChar *str, SysChar oldchar, SysChar newchar) {
  sys_return_val_if_fail(str != NULL, false);

  SysChar *s = str;
  while (*str) {
    if (*str == oldchar) {
      *s = newchar;
    } else {
      *s = *str;
    }
    s++;
    str++;
  }
  *s = '\0';

  return true;
}

SysDouble sys_str_to_double(const SysChar *str) {
  sys_return_val_if_fail(str != NULL, 0);
  return strtod(str, NULL);
}

SysInt64 sys_str_to_int64(const SysChar *str) {
  sys_return_val_if_fail(str != NULL, 0);

  return strtol(str, NULL, 10);
}

/**
 * sys_str_newsize: new bytes of str terminated with '\0', for init.
 * @size: size of byte.
 *
 * Returns: void
 */
SysChar* sys_str_newsize(SysSSize size) {
  sys_return_val_if_fail(size > 1, NULL);

  SysChar *nstr;

  if (size > SYS_BYTE_MAX) {
    size = SYS_BYTE_INIT_SIZE;
  }

  nstr = sys_new0(SysChar, size + 1);

  return nstr;
}

/**
 * sys_strstr_len:
 * @haystack: a nul-terminated string
 * @haystack_len: the maximum length of @haystack in bytes. A length of -1
 *     can be used to mean "search the entire string", like `strstr()`.
 * @needle: the string to search for
 *
 * Searches the string @haystack for the first occurrence
 * of the string @needle, limiting the length of the search
 * to @haystack_len or a nul terminator byte (whichever is reached first).
 *
 * Returns: a pointer to the found occurrence, or
 *    %NULL if not found.
 */
SysChar * sys_strstr_len (const SysChar *haystack,
    SysSSize       haystack_len,
    const SysChar *needle) {
  sys_return_val_if_fail (haystack != NULL, NULL);
  sys_return_val_if_fail (needle != NULL, NULL);

  if (haystack_len < 0)
    return strstr (haystack, needle);
  else
  {
    const SysChar *p = haystack;
    SysSSize needle_len = strlen (needle);
    SysSSize haystack_len_unsigned = haystack_len;
    const SysChar *end;
    SysSSize i;

    if (needle_len == 0)
      return (SysChar *)haystack;

    if (haystack_len_unsigned < needle_len)
      return NULL;

    end = haystack + haystack_len - needle_len;

    while (p <= end && *p)
    {
      for (i = 0; i < needle_len; i++)
        if (p[i] != needle[i])
          goto next;

      return (SysChar *)p;

next:
      p++;
    }

    return NULL;
  }
}

/**
 * sys_strrstr:
 * @haystack: a nul-terminated string
 * @needle: the nul-terminated string to search for
 *
 * Searches the string @haystack for the last occurrence
 * of the string @needle.
 *
 * Returns: a pointer to the found occurrence, or
 *    %NULL if not found.
 */
SysChar * sys_strrstr (const SysChar *haystack,
    const SysChar *needle) {
  SysSSize i;
  SysSSize needle_len;
  SysSSize haystack_len;
  const SysChar *p;

  sys_return_val_if_fail (haystack != NULL, NULL);
  sys_return_val_if_fail (needle != NULL, NULL);

  needle_len = strlen (needle);
  haystack_len = strlen (haystack);

  if (needle_len == 0)
    return (SysChar *)haystack;

  if (haystack_len < needle_len)
    return NULL;

  p = haystack + haystack_len - needle_len;

  while (p >= haystack)
  {
    for (i = 0; i < needle_len; i++)
      if (p[i] != needle[i])
        goto next;

    return (SysChar *)p;

next:
    p--;
  }

  return NULL;
}

/**
 * sys_strrstr_len:
 * @haystack: a nul-terminated string
 * @haystack_len: the maximum length of @haystack in bytes. A length of -1
 *     can be used to mean "search the entire string", like sys_strrstr().
 * @needle: the nul-terminated string to search for
 *
 * Searches the string @haystack for the last occurrence
 * of the string @needle, limiting the length of the search
 * to @haystack_len.
 *
 * Returns: a pointer to the found occurrence, or
 *    %NULL if not found.
 */
SysChar * sys_strrstr_len (const SysChar *haystack,
    SysSSize        haystack_len,
    const SysChar *needle) {
  sys_return_val_if_fail (haystack != NULL, NULL);
  sys_return_val_if_fail (needle != NULL, NULL);

  if (haystack_len < 0) {

    return sys_strrstr (haystack, needle);
  }
  else
  {
    SysSSize needle_len = strlen (needle);
    const SysChar *haystack_max = haystack + haystack_len;
    const SysChar *p = haystack;
    SysSSize i;

    while (p < haystack_max && *p)
      p++;

    if (p < haystack + needle_len)
      return NULL;

    p -= needle_len;

    while (p >= haystack)
    {
      for (i = 0; i < needle_len; i++)
        if (p[i] != needle[i])
          goto next;

      return (SysChar *)p;

next:
      p--;
    }

    return NULL;
  }
}

int sys_ascii_digit_value (SysChar c) {
  if (isdigit (c))
    return c - '0';
  return -1;
}

int sys_ascii_xdigit_value (SysChar c) {
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  return sys_ascii_digit_value (c);
}

SysInt sys_ascii_strcasecmp (
    const SysChar *s1,
    const SysChar *s2) {
  SysInt c1, c2;

  sys_return_val_if_fail (s1 != NULL, 0);
  sys_return_val_if_fail (s2 != NULL, 0);

  while (*s1 && *s2) {

      c1 = (SysInt)(SysUChar) tolower (*s1);
      c2 = (SysInt)(SysUChar) tolower (*s2);
      if (c1 != c2)
        return (c1 - c2);
      s1++; s2++;
    }

  return (((SysInt)(SysUChar) *s1) - ((SysInt)(SysUChar) *s2));
}
