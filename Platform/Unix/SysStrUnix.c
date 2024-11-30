#include <System/Utils/SysStr.h>

SysSize sys_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp) {
  return fread(buffer, bufsize, elem_size * elem_count, fp);
}

SysInt sys_vprintf(const SysChar *format, va_list va) {
  return vprintf(format, va);
}

void sys_strcpy(SysChar* __restrict dst, const SysChar* __restrict src) {
  strcpy(dst, src);
}

SysChar* sys_strncpy(SysChar* __restrict dst, const SysChar* __restrict src, SysSize n) {
  return strncpy(dst, src, n);
}

SysInt sys_vasprintf (SysChar** str, const SysChar* format, va_list va) {
  va_list vap;
  SysInt count;
  SysChar * buff;

  *str = NULL;

  va_copy(vap, va);
  count = vsnprintf(NULL, 0, format, va);
  if (count >= 0) {

    buff = sys_malloc(count + 1);
    if (buff) {

      count = vsnprintf(buff, count + 1, format, vap);
      if (count < 0) {
        sys_free(buff);

      } else {
        *str = buff;
      }
    }
  }
  va_end(vap);

  return count;
}

SysInt sys_vsprintf(SysChar* str, SysSize size, const SysChar* format, va_list args) {
  return vsnprintf(str, size, format, args);
}

SysChar *sys_wchar_to_mbyte(const SysWChar *wchar, SysInt *nsize) {
  SysInt size;
  SysChar* sz;

  setlocale(LC_CTYPE, "");
  size = wcstombs(NULL, wchar, 0) + 1;
  sz = (SysChar* )sys_malloc(size * sizeof(SysChar));
  wcstombs(sz, wchar, size);
  if (nsize) {
    *nsize = size;
  }

  return sz;
}

SysWChar *sys_mbyte_to_wchar(const SysChar *mbyte, SysInt *nsize) {
  SysWChar *sz;
  SysSize mbslen;

  setlocale(LC_CTYPE, "");
  mbslen = mbstowcs(NULL, mbyte, 0);
  if (mbslen == (SysSize)(-1)) {
    perror("sys_mbyte_to_wchar failed");
    exit(-1);
  }

  sz = (SysWChar *)sys_malloc((mbslen + 1) * sizeof(SysWChar));
  if (mbstowcs(sz, mbyte, mbslen + 1) == (SysSize) -1) {
    perror("mbstowcs");
    exit(EXIT_FAILURE);
  }
  if (nsize) {
    *nsize = mbslen + 1;
  }

  return sz;
}
