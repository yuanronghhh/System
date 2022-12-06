#include <DataTypes/SysArray.h>
#include <Utils/SysString.h>
#include <Utils/SysPathPrivate.h>

SysChar *sys_real_getcwd(void) {
  SysChar buf[MAX_PATH];
  SysChar *cwd;

  cwd = getcwd (buf, MAX_PATH);
  return cwd;
}

bool sys_real_path_exists(const SysChar *path) {
  if(access (path, F_OK) == 0) {
    return true;
  }

  return false;
}
