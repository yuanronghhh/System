#ifndef __SYS_STRING_PRIVATE_H__
#define __SYS_STRING_PRIVATE_H__

#include <System/Utils/SysString.h>

SYS_BEGIN_DECLS

SysWChar *sys_real_ansi_to_wchar(const SysChar *ansi);
SysChar *sys_real_wchar_to_ansi(const SysWChar *uni);
void sys_real_strcpy(SysChar* __restrict dst, const SysChar* __restrict src);
SysChar *sys_real_strncpy(SysChar *dst, SysSize n, const SysChar *src);
int sys_real_vprintf(const SysChar *format, va_list va);
int sys_real_vsprintf(SysChar* str, SysSize size, const SysChar* format, va_list args);
int sys_real_vasprintf(SysChar** ptr, const SysChar* format, va_list va);

SYS_END_DECLS

#endif
