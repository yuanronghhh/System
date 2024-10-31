#ifndef __SYS_PATH_H__
#define __SYS_PATH_H__

#include <System/Fundamental/SysCommonCore.h>
#include <System/Utils/SysStr.h>

SYS_BEGIN_DECLS

#define sys_path_join(...) _sys_strjoin("/", __VA_ARGS__, NULL)
#define SYS_IS_DIR_SEPARATOR(c) ((c) == PATH_SEP || (c) == '/')
#define SYS_DIR_SEPARATOR PATH_SEP

#define sys_path_build_filename sys_path_join
SYS_API SysChar* sys_path_name(const SysChar* path);
SYS_API SysBool sys_path_exists(const SysChar* path);
SYS_API const SysChar* sys_path_basename(const SysChar* path);
SYS_API const SysChar *sys_path_extension(const SysChar *path);
SYS_API SysChar* sys_path_purename(const SysChar *path);
SYS_API SysBool sys_path_escape(SysChar* buf);
SYS_API SysChar* sys_path_dirname(const SysChar* path);
SYS_API SysChar* sys_path_getcwd(void);
#define sys_getcwd sys_path_getcwd
SYS_API SysBool sys_path_is_absolute(const SysChar *path);
SYS_API const SysChar * sys_path_skip_root (const SysChar *file_name);
SYS_API SysChar * sys_path_canonicalize_filename (const SysChar *filename,
    const SysChar *relative_to);

SYS_END_DECLS

#endif
