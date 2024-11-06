#ifndef __SYS_LIB_CHARSET_H__
#define __SYS_LIB_CHARSET_H__

#include <System/Fundamental/SysCommonBase.h>

SYS_BEGIN_DECLS

/* Determine the current locale's character encoding, and canonicalize it
   into one of the canonical names listed in config.charset.
   The result must not be freed; it is statically allocated.
   If the canonical name cannot be determined, the result is a non-canonical
   name.  */
const char * _sys_locale_charset_raw (void);
const char * _sys_locale_charset_unalias (const char *codeset);
const char * _sys_locale_get_charset_aliases (void);

SYS_END_DECLS

#endif
