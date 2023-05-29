#include <System/DataTypes/SysArray.h>
#include <System/Utils/SysString.h>
#include <System/Utils/SysPathPrivate.h>


SysChar *sys_real_getcwd(void) {
  wchar_t wbuf[MAX_PATH];
  DWORD len;
  SysChar *cwd;

  len = GetCurrentDirectoryW(ARRAY_SIZE(wbuf), wbuf);
  if (len >= ARRAY_SIZE(wbuf)) {
    sys_warning_N("sys_getcwd GetCurrentDirectoryW Failed.");
    return NULL;
  }

  cwd = sys_wchar_to_ansi(&wbuf[0]);
  return cwd;
}

bool sys_real_path_exists(const SysChar *path) {
  SysWChar *wname = sys_ansi_to_wchar(path);
  DWORD attr;

  attr = GetFileAttributesW(wname);
  sys_free_N(wname);

  if (attr == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return true;
}
