#ifndef __SYS_PATH_PRIVATE_H__
#define __SYS_PATH_PRIVATE_H__

#include <System/Utils/SysPath.h>

SYS_BEGIN_DECLS

SysChar *sys_real_getcwd(void);
SysBool sys_real_path_exists(const SysChar *path);
SysBool sys_real_path_is_absolute(const SysChar *path);

SYS_END_DECLS

#endif
