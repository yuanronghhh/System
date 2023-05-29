#ifndef __SYS_PROCESS_H__
#define __SYS_PROCESS_H__

#include <System/Fundamental/SysCommon.h>

typedef struct _SysSubProcess SysSubProcess;
typedef struct _SysSubProcessOption SysSubProcessOption;

struct _SysSubProcessOption {
  const SysChar **cmd;
  SysSize cmdlen;
  const SysChar *cwd;

  SYS_IO_ENUM std0type;
  SYS_IO_ENUM std1type;
  SYS_IO_ENUM std2type;
};

struct _SysSubProcess {
  SysSubProcessOption option;
};

SYS_API SysInt sys_execvp(const SysChar* path, SysChar* const args[]);
SYS_API FILE *sys_popen(const SysChar* cmd, const SysChar *mode);
SYS_API SysInt sys_pclose(FILE *fp);

SYS_API SysSubProcess* sys_subprocess_new(const SysChar *cmd[]);
SYS_API SysSubProcess* sys_subprocess_new_option(SysSubProcessOption* option);
SYS_API bool sys_subprocess_communicate(SysSubProcess *sub,
    SysChar*         stdin_buf,
    SysChar**        stdout_buf,
    SysChar**        stderr_buf,
    SysError**       error);

SYS_API void sys_subprocess_terminate(SysSubProcess *sub);

#endif
