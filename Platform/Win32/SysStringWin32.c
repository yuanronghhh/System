#include <System/Utils/SysError.h>
#include <System/Utils/SysString.h>

SysChar *sys_wchar_to_mbyte(const SysWChar *wchar) {
  int size;
  SysChar *nstr;

  size = WideCharToMultiByte(CP_ACP, 0, wchar, -1, NULL, 0, NULL, NULL);
  nstr = (SysChar *)malloc(size * sizeof(SysChar));
  WideCharToMultiByte(CP_ACP, 0, wchar, -1, nstr, size, NULL, NULL);

  return nstr;
}

SysWChar *sys_mbyte_to_wchar(const SysChar *mbyte) {
  int size;
  SysWChar *wstr;

  size = MultiByteToWideChar(CP_ACP, 0, mbyte, -1, NULL, 0);
  wstr = (SysWChar *)malloc(size * sizeof(SysWChar));
  MultiByteToWideChar(CP_ACP, 0, mbyte, -1, wstr, size);

  return wstr;
}

void sys_strcpy(SysChar *__restrict dst, const SysChar *__restrict src) {
  SysSize n = strlen(src);
  memcpy(dst, src, n);
  dst[n] = '\0';
}

SysChar *sys_strncpy(SysChar * __restrict dst, SysSize n, const SysChar * __restrict src) {
  memcpy(dst, src, n);
  dst[n] = '0';
  return dst;
}

int sys_vsprintf(SysChar *str, SysSize size, const SysChar *format, va_list args) {
  return vsprintf_s(str, size, format, args);
}

int sys_vprintf(const SysChar *format, va_list va) {
  return vprintf_s(format, va);
}

int sys_vasprintf(SysChar **ptr, const SysChar *format, va_list va) {
  int len;

  len = _vscprintf_p(format, va) + 1;
  *ptr = sys_new_N(SysChar, len);
  if (!*ptr) {
    return -1;
  }

  return _vsprintf_p(*ptr, len, format, va);
}
