#ifndef __SYS_PATH_H__
#define __SYS_PATH_H__

#include <Fundamental/SysCommonCore.h>
#include <Utils/SysString.h>

SYS_BEGIN_DECLS

#define sys_path_join(...) _sys_strjoin("/", __VA_ARGS__, NULL)

SYS_API SysChar* sys_path_name(const SysChar* path);
SYS_API bool sys_path_exists(const SysChar* path);
SYS_API const SysChar* sys_path_basename(const SysChar* path);
SYS_API const SysChar *sys_path_extension(const SysChar *path);
SYS_API SysChar* sys_path_purename(const SysChar *path);
SYS_API bool sys_path_escape(SysChar* buf);
SYS_API SysChar* sys_path_dirname(const SysChar* path);
SYS_API SysChar* sys_getcwd(void);

SYS_END_DECLS

#endif
