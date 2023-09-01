#include <System/Utils/SysString.h>
#include <System/Utils/SysError.h>
#include <System/Platform/Common/SysOs.h>
#include <System/Platform/Common/SysMem.h>

#define SYS_BYTE_INIT_SIZE 4096

/**
 * sys_strsplit: split with delimiter
 * @s: dst string
 * @delim: seperate char
 * @count: split count.
 *
 * Returns: void
 */
SysChar **sys_strsplit(SysChar * s, const SysChar *delim, int * count) {
  void *data;
  SysChar* _s = (SysChar*)s;
  const SysChar ** ptrs;
  SysSize
    ptrsSize,
    sLen = strlen(s),
    delLen = strlen(delim);
  *count = 1;

  while ((_s = strstr(_s, delim)) != NULL) {
    _s += delLen;
    ++*count;
  }

  ptrsSize = (*count + 1) * sizeof(SysChar *);
  ptrs = data = sys_malloc_N(ptrsSize + sLen + 1);
  if (data) {
    sys_strcpy(((SysChar *)data) + ptrsSize, s);
    *ptrs = _s = ((SysChar *)data) + ptrsSize;

    if (*count > 1) {
      while ((_s = strstr(_s, delim)) != NULL) {
        *_s = '\0';
        _s += delLen;
        *++ptrs = _s;
      }
    }
    *++ptrs = NULL;
  }

  return data;
}

/**
 * sys_strcat_M:
 *   copy v2 to v1, return v1, realloc v1 if v1 not enough.
 *   simple method for concat string, use sys_string_* for better api.
 *
 *  example:
 *    SysSize mlen = 200, len = 0;
 *    SysChar *s = sys_str_newsize(char, mlen);
 *    sys_strmcat(&s, &mlen, &len, "var");
 *    sys_strmcat(&s, &mlen, &len, "=");
 *    sys_strmcat(&s, &mlen, &len, "foo");
 *    sys_free_N(s);
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

    nstr = sys_realloc_N(*v1, nearup);
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
 * sys_strlcat: copy v2 to v1, return v1, return v1 if v1 not enough.
 * @v1:
 * @v1_max:
 * @v2:
 *
 * Returns: void
 */
SysChar* sys_strlcat(SysChar* v1, SysSize v1_max, const SysChar* v2) {
  sys_return_val_if_fail(v1 != NULL, NULL);
  sys_return_val_if_fail(v2 != NULL, NULL);

  SysSize v1_len = sys_strlen(v1, v1_max);
  SysSize v2_len = strlen(v2);
  SysSize nlen = v1_len + v2_len;

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

  SysSize len = strlen(s);
  SysChar *n = sys_new_N(SysChar, len + 1);
  memcpy(n, s, len);
  n[len] = '\0';

  return n;
}

/**
 * sys_strndup: copy length of s.
 *
 * Returns: new alloc str.
 */
SysChar* sys_strndup(const SysChar *s, SysSize len) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysChar *n = sys_new_N(SysChar, len + 1);
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
int sys_asprintf(SysChar **str, const SysChar *format, ...) {
  sys_return_val_if_fail(*str == NULL && "sys_asprintf *str should be NULL", -1);

  int len;

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
int sys_printf(const SysChar *format, ...) {
  int len;

  va_list args;
  va_start(args, format);

  len = sys_vprintf(format, args);

  va_end(args);

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
  SYS_LEAK_IGNORE_END;
  fflush(stdout);
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
int sys_snprintf(SysChar *str, SysSize size, const SysChar *format, ...) {
  int len;

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
SysChar* sys_strjoin_array(const SysChar *delim, const SysChar **arr, SysSize *dstlen) {
  sys_return_val_if_fail(delim != NULL, NULL);
  sys_return_val_if_fail(arr != NULL, NULL);

  SysChar *nstr, *ptr;
  SysSize len;
  SysSize dlen = strlen(delim);

  const SysChar **na = arr;
  if (!(*na)) {
    nstr = sys_strdup("");
    return nstr;
  }

  len = strlen((*na));

  while(++(*na)) {
    len += dlen + strlen((*na));
  }

  nstr = sys_new0_N(SysChar, len + 1);
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
  SysSize len;
  SysSize dlen;
  va_list nargs;

  dlen = strlen(delim);

  va_copy(nargs, args);
  s = va_arg(nargs, SysChar *);
  len = strlen(s);

  for (s = va_arg(nargs, SysChar *); s;) {
    len += dlen + strlen(s);
    s = va_arg(nargs, SysChar *);
  }

  nstr = sys_new0_N(SysChar, len + 1);

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
 *    int a = 16;
 *    char *buf = sys_bin_new(a, sizeof(a));
 *    buf = sys_bin_tostr(buf, a, sizeof(a));
 *
 * @buffer: data store buffer.
 * @bufsize: 8 * sizeof(type) + 1
 * @sbyte: sizeof(type)
 * @ptr: target pointer
 *
 * Returns: buffer
 */
SysChar* sys_bin_str_full(SysChar *buffer, SysSize bufsize, SysSize const sbyte, void const *const ptr) {
  SysUChar *b = (SysUChar *)ptr;
  SysUChar byte;

  if (bufsize != (sbyte * 8 + 1)) {
    perror("sys_bin_tostr bufsize not equal sbyte * 8 + 1.");
    abort();
  }

  int i, j, m = 0;
  for (i = (int)sbyte - 1; i >= 0; i--) {
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
SysSize sys_strlen(const SysChar *s, SysSize max) {
  SysSize len;

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

int sys_strcmp(const SysChar *s1, const SysChar *s2) {
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

int sys_strncmp(const SysChar *s1, const SysChar *s2, int max) {
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
bool sys_str_override_c(SysChar *str, char oldchar, char newchar) {
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
SysChar* sys_str_newsize(SysSize size) {
  sys_return_val_if_fail(size > 1, NULL);

  SysChar *nstr;

  if (size > SYS_BYTE_MAX) {
    size = SYS_BYTE_INIT_SIZE;
  }

  nstr = sys_new0_N(SysChar, size + 1);

  return nstr;
}
