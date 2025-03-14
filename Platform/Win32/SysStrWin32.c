#include <System/Utils/SysError.h>
#include <System/Utils/SysStr.h>

SysChar *sys_wchar_to_mbyte(const SysWChar *wchar, SysInt *nsize) {
  SysInt size;
  SysChar *nstr;

  size = WideCharToMultiByte(CP_UTF8, 0, wchar, -1, NULL, 0, NULL, NULL);
  nstr = (SysChar *)sys_malloc0((SysSSize)(size + 1) + sizeof(SysChar));
  nstr[size] = '\0';

  WideCharToMultiByte(CP_UTF8, 0, wchar, -1, nstr, size, NULL, NULL);
  if (nsize) {
    *nsize = size + 1;
  }

  return nstr;
}

SysWChar *sys_mbyte_to_wchar(const SysChar *mbyte, SysInt *nsize) {
  SysInt size;
  SysWChar *wstr;

  size = MultiByteToWideChar(CP_UTF8, 0, mbyte, -1, NULL, 0);
  wstr = (SysWChar *)sys_malloc0((SysSSize)(size + 1) * sizeof(SysWChar));
  MultiByteToWideChar(CP_UTF8, 0, mbyte, -1, wstr, size);

  if (nsize) {
    *nsize = size + 1;
  }

  return wstr;
}

void sys_strcpy(SysChar *__restrict dst, const SysChar *__restrict src) {
  SysSSize n = strlen(src);
  memcpy(dst, src, n);
  dst[n] = '\0';
}

SysChar *sys_strncpy(SysChar * __restrict dst, const SysChar * __restrict src, SysSize n) {
  memcpy(dst, src, n);
  dst[n] = '\0';
  return dst;
}

SysInt sys_vsprintf(SysChar *str, SysSize size, const SysChar *format, va_list args) {
  return vsprintf_s(str, size, format, args);
}

SysInt sys_vprintf(const SysChar *format, va_list va) {
  return vprintf_s(format, va);
}

SysInt sys_vasprintf(SysChar **ptr, const SysChar *format, va_list va) {
  SysInt len;

  len = _vscprintf_p(format, va) + 1;
  *ptr = sys_new(SysChar, len);
  if (!*ptr) {
    return -1;
  }

  return _vsprintf_p(*ptr, len, format, va);
}
