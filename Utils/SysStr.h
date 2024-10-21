#ifndef __SYS_STRING_H__
#define __SYS_STRING_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define SYS_BYTE_MAX INT_MAX
#define SYS_LSTR(str) (SysChar *)(str), (sizeof(str) - 1)
#define sys_bin_new(o, szof) sys_malloc0((szof) * 8 + 1)
#define sys_bin_tostr(buffer, o, szof) sys_bin_str_full(buffer, (szof) * 8 + 1, szof, &o)
#define sys_strneq(s1, s2, n) (strncmp(s1, s2, (n)) == 0)
#define sys_str_startswith(s1, s2) (strncmp(s1, s2, sizeof(s2)-1) == 0)
#define sys_strjoin(delim, ...) _sys_strjoin(delim, __VA_ARGS__, NULL)
#define SYS_STR_MULTILINE(...) #__VA_ARGS__

SYS_API SysWChar * sys_mbyte_to_wchar(const SysChar * mbyte, SysInt * nsize);
SYS_API SysChar* sys_wchar_to_mbyte(const SysWChar* wchar, SysInt * nsize);

SYS_API SysDouble sys_str_to_double(const SysChar* str);
SYS_API SysInt64 sys_str_to_int64(const SysChar* str);

SYS_API SysChar* sys_bin_str_full(SysChar* buffer, SysSize bufsize, SysSize const sbyte, void const* const ptr);
SYS_API SysChar* sys_strjoin_array(const SysChar* delim, const SysChar** arr, SysSize* dstlen);
SYS_API SysChar* _sys_strjoin(const SysChar* delim, ...);
SYS_API SysChar* sys_strjoinv(const SysChar* delim, va_list args);
SYS_API SysInt sys_snprintf(SysChar* str, SysSize size, const SysChar* format, ...);
SYS_API SysInt sys_vprintf(const SysChar* format, va_list va);
SYS_API SysInt sys_printf(const SysChar* format, ...);
SYS_API SysInt sys_vasprintf(SysChar** str, const SysChar* format, va_list va);
SYS_API SysInt sys_asprintf(SysChar** str, const SysChar* format, ...);
SYS_API SysChar* sys_strdup_printf(const SysChar* format, ...);
SYS_API void sys_print(const SysChar* str);
SYS_API void sys_strcpy(SysChar* __restrict dst, const SysChar* __restrict src);
SYS_API SysChar* sys_strncpy(SysChar* __restrict dst, const SysChar* __restrict src, SysSize n);
SYS_API SysSize sys_strlen(const SysChar* s, SysSize max);
SYS_API SysBool sys_str_equal(const SysChar* s1, const SysChar* s2);
SYS_API SysInt sys_strcmp(const SysChar* s1, const SysChar* s2);
SYS_API SysInt sys_strncmp(const SysChar* s1, const SysChar* s2, SysInt max);
SYS_API SysChar* sys_strupper(SysChar* s);
SYS_API SysChar* sys_strlower(SysChar* s);
SYS_API SysChar* sys_strndup(const SysChar* s, SysSize len);
SYS_API SysChar* sys_strdup(const SysChar* s);
SYS_API SysChar* sys_strpncpy(SysChar* dst, SysInt n, const SysChar* src);
SYS_API SysChar* sys_strpcpy(SysChar* dst, const SysChar* src);
SYS_API SysChar* sys_str_newsize(SysSize size);
SYS_API void sys_strmcat(SysChar** v1, SysSize* v1_max, SysSize* len, const SysChar* v2);
SYS_API SysChar* sys_strlcat(SysChar* v1, SysSize v1_max, const SysChar* v2);
SYS_API SysInt sys_vsprintf(SysChar* str, SysSize size, const SysChar* format, va_list args);
SYS_API void sys_str_trim_end(SysChar *s, const SysChar delim);
SYS_API SysChar* sys_strstr(const SysChar *s, const SysChar* delim);

SYS_API SysBool sys_str_override_c(SysChar* str, SysChar oldchar, SysChar newchar);
SYS_API SysChar** sys_strsplit(SysChar*  s, const SysChar* delim, SysInt*  count);

SYS_END_DECLS

#endif
