#ifndef __SYS_CONVERT_PRIVATE_H__
#define __SYS_CONVERT_PRIVATE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

SysChar *_sys_time_locale_to_utf8 (const SysChar *opsysstring,
                               SysSize len,
                               SysSize *bytes_read,
                               SysSize *bytes_written,
                               SysError **error) SYS_GNUC_MALLOC;

SysChar *_sys_ctype_locale_to_utf8 (const SysChar *opsysstring,
                                SysSize len,
                                SysSize *bytes_read,
                                SysSize *bytes_written,
                                SysError **error) SYS_GNUC_MALLOC;

SYS_END_DECLS

#endif
