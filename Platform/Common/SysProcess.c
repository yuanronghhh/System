#include <System/Utils/SysString.h>
#include <System/Platform/Common/SysProcessPrivate.h>

SysInt sys_execvp(const SysChar* path, const SysChar* const args[]) {
  return sys_real_execvp(path, args);
}

FILE *sys_popen(const SysChar* cmd, const SysChar *mode) {
  return sys_real_popen(cmd, mode);
}

SysInt sys_pclose(FILE *fp) {
  return sys_real_pclose(fp);
}

SysSubProcess* sys_subprocess_new(const SysChar *cmd[]) {
  return sys_real_subprocess_new(cmd);
}

SysSubProcess* sys_subprocess_new_option(SysSubProcessOption* option) {
  SysSubProcess *sub;

  sub = sys_real_subprocess_new_option(option);

  return sub;
}

SYS_API SysBool sys_subprocess_communicate(SysSubProcess *sub,
    SysChar*   stdin_buf,
    SysChar**  stdout_buf,
    SysChar**  stderr_buf,
    SysError** error) {
  sys_return_val_if_fail(sub != NULL, false);
  sys_return_val_if_fail(stdin_buf != NULL, false);
  sys_return_val_if_fail(*stdout_buf == NULL, false);
  sys_return_val_if_fail(*stderr_buf == NULL, false);
  sys_return_val_if_fail(*error == NULL, false);

  sys_error_N("%s", "Not Implement.");

  return false;
}

void sys_subprocess_terminate(SysSubProcess *sub) {
  sys_real_subprocess_terminate(sub);
}
