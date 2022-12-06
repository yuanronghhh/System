#ifndef __SYS_PROCESS_PRIVATE_H__
#define __SYS_PROCESS_PRIVATE_H__

#include <Platform/SysProcess.h>

SysInt sys_real_dev_null(void);
SysInt sys_real_execvp(const SysChar* path, SysChar* const args[]);
FILE *sys_real_popen(const SysChar* cmd, const SysChar *mode);
SysInt sys_real_pclose(FILE *fp);

SysSubProcess* sys_real_subprocess_new(const SysChar *cmd[]);
SysSubProcess* sys_real_subprocess_new_option(SysSubProcessOption* option);
void sys_real_subprocess_terminate(SysSubProcess *sub);

#endif
