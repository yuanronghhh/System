#include <System/DataTypes/SysArray.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysPathPrivate.h>

SysChar *sys_real_path_getcwd(void) {
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

const SysChar * sys_path_skip_root (const SysChar *file_name) {
  sys_return_val_if_fail (file_name != NULL, NULL);

  /* Skip initial slashes */
  if (SYS_IS_DIR_SEPARATOR (file_name[0])) {
    while (SYS_IS_DIR_SEPARATOR (file_name[0]))
      file_name++;
    return (SysChar *)file_name;
  }

  return NULL;
}
