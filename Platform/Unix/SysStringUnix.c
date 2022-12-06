#include <Utils/SysString.h>

SysSize sys_real_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp) {
  return fread(buffer, bufsize, elem_size * elem_count, fp);
}

int sys_real_vprintf(const SysChar *format, va_list va) {
  return vprintf(format, va);
}

void sys_real_strcpy(SysChar* __restrict dst, const SysChar* __restrict src) {
  strcpy(dst, src);
}

SysChar* sys_real_strncpy(SysChar* dest, SysSize n, const SysChar* src) {
  return strncpy(dest, src, n);
}

SysInt sys_real_vasprintf (SysChar** str, const SysChar* format, va_list va) {
  va_list vap;
  SysInt count;
  SysChar * buff;

  *str = NULL;

  va_copy(vap, va);
  count = vsnprintf(NULL, 0, format, va);
  if (count >= 0) {

    buff = sys_malloc_N(count + 1);
    if (buff) {

      count = vsnprintf(buff, count + 1, format, vap);
      if (count < 0) {
        sys_free_N(buff);

      } else {
        *str = buff;
      }
    }
  }
  va_end(vap);

  return count;
}

SysInt sys_real_vsprintf(SysChar* str, SysSize size, const SysChar* format, va_list args) {
  return vsnprintf(str, size, format, args);
}

SysChar* sys_real_wchar_to_ansi(const SysWChar *uni) {
  SysInt size;
  SysChar* sz;

  setlocale(LC_CTYPE, "");
  size = wcstombs(NULL, uni, 0) + 1;
  sz = (SysChar* )malloc(size * sizeof(char));
  wcstombs(sz, uni, size);

  return sz;
}

SysWChar *sys_real_ansi_to_wchar(SysChar* ansi) {
  SysWChar *sz;
  SysSize mbslen;

  setlocale(LC_CTYPE, "");
  mbslen = mbstowcs(NULL, ansi, 0);
  if (mbslen == (SysSize)-1) {
    perror("sys_real_ansi_to_wchar failed");
    exit(-1);
  }

  sz = (SysWChar *)malloc((mbslen + 1) * sizeof(SysWChar));
  if (mbstowcs(sz, ansi, mbslen + 1) == (SysSize) -1) {
    perror("mbstowcs");
    exit(EXIT_FAILURE);
  }

  return sz;
}
