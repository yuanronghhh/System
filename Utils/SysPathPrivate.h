#ifndef __SYS_PATH_PRIVATE_H__
#define __SYS_PATH_PRIVATE_H__

#include <Utils/SysPath.h>

SYS_BEGIN_DECLS

SysChar *sys_real_getcwd(void);
bool sys_real_path_exists(const SysChar *path);

SYS_END_DECLS

#endif
