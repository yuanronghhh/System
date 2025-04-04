#include <System/DataTypes/SysArray.h>
#include <System/Utils/SysString.h>
#include <System/Utils/SysPathPrivate.h>


SysChar *sys_real_getcwd(void) {
  wchar_t wbuf[MAX_PATH];
  DWORD len;
  SysChar *cwd;

  len = GetCurrentDirectoryW(ARRAY_SIZE(wbuf), wbuf);
  if (len >= ARRAY_SIZE(wbuf)) {
    sys_warning_N("%s", "sys_getcwd GetCurrentDirectoryW Failed.");
    return NULL;
  }

  cwd = sys_wchar_to_mbyte(&wbuf[0], NULL);
  return cwd;
}

SysBool sys_real_path_exists(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, false);

  SysWChar *wname = sys_mbyte_to_wchar(path, NULL);
  DWORD attr;

  attr = GetFileAttributesW(wname);
  sys_free(wname);

  if (attr == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return true;
}

SysBool sys_real_path_is_absolute(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, false);

  if (*path == '/' || *path == '\\') {
    return true;
  }

  if (*path != '\0' && *(++path) != ':') {
    return true;
  }

  return false;
}
