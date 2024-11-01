#ifndef __SYS_CHARSET_H__
#define __SYS_CHARSET_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

SysBool              sys_get_charset         (const char **charset);
SysChar *               sys_get_codeset         (void);
SysBool              sys_get_console_charset (const char **charset);

const SysChar * const * sys_get_language_names  (void);
const SysChar * const * sys_get_language_names_with_category
                                            (const SysChar *category_name);
SysChar **              sys_get_locale_variants (const SysChar *locale);

SYS_END_DECLS

#endif
