#include <System/DataTypes/SysArray.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysPathPrivate.h>

SysChar *sys_real_getcwd(void) {
  SysChar buf[MAX_PATH];
  SysChar *cwd;

  cwd = getcwd (buf, MAX_PATH);
  return cwd;
}

SysBool sys_real_path_exists(const SysChar *path) {
  if(access (path, F_OK) == 0) {
    return true;
  }

  return false;
}

SysBool sys_real_path_is_absolute(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, false);

  if (*path == '/') {
    return true;
  }

  return false;
}
