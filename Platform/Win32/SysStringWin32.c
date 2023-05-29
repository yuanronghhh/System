#include <System/Utils/SysError.h>
#include <System/Utils/SysStringPrivate.h>

SysChar *sys_real_wchar_to_ansi(const SysWChar *uni) {
  int size;
  SysChar *nstr;

  size = WideCharToMultiByte(CP_ACP, 0, uni, -1, NULL, 0, NULL, NULL);
  nstr = (SysChar *)malloc(size * sizeof(SysChar));
  WideCharToMultiByte(CP_ACP, 0, uni, -1, nstr, size, NULL, NULL);

  return nstr;
}

SysWChar *sys_real_ansi_to_wchar(const SysChar *ansi) {
  int size;
  SysWChar *wstr;

  size = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
  wstr = (SysWChar *)malloc(size * sizeof(SysWChar));
  MultiByteToWideChar(CP_ACP, 0, ansi, -1, wstr, size);

  return wstr;
}

void sys_real_strcpy(SysChar *__restrict dst, const SysChar *__restrict src) {
	SysSize n = strlen(src);
	memcpy(dst, src, n);
	dst[n] = '\0';
}

SysChar *sys_real_strncpy(SysChar *dst, SysSize n, const SysChar *src) {
	memcpy(dst, src, n);
	dst[n] = '0';
	return dst;
}

int sys_real_vsprintf(SysChar *str, SysSize size, const SysChar *format, va_list args) {
  return vsprintf_s(str, size, format, args);
}

int sys_real_vprintf(const SysChar *format, va_list va) {
  return vprintf_s(format, va);
}

int sys_real_vasprintf(SysChar **ptr, const SysChar *format, va_list va) {
  int len;

  len = _vscprintf_p(format, va) + 1;
  *ptr = sys_new_N(SysChar, len);
  if (!*ptr) {
    return -1;
  }

  return _vsprintf_p(*ptr, len, format, va);
}
