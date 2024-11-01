#ifndef __SYS_CHARSET_PRIVATE_H__
#define __SYS_CHARSET_PRIVATE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

const char ** _sys_charset_get_aliases (const char *canonical_name);

SysBool      _sys_get_time_charset    (const char **charset);

SysBool      _sys_get_ctype_charset   (const char **charset);

SYS_END_DECLS

#endif
