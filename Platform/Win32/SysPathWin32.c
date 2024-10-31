#include <System/DataTypes/SysArray.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysPathPrivate.h>

SysChar *sys_real_path_getcwd(void) {
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
  SysWChar *wname;
  DWORD attr;

  wname = sys_mbyte_to_wchar(path, NULL);
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

const SysChar * sys_path_skip_root (const SysChar *file_name) {
  sys_return_val_if_fail (file_name != NULL, NULL);

  /* Skip \\server\share or //server/share */
  if (SYS_IS_DIR_SEPARATOR (file_name[0]) &&
      SYS_IS_DIR_SEPARATOR (file_name[1]) &&
      file_name[2] &&
      !SYS_IS_DIR_SEPARATOR (file_name[2]))
    {
      SysChar *p;
      p = strchr (file_name + 2, SYS_DIR_SEPARATOR);

      {
        SysChar *q;

        q = strchr (file_name + 2, '/');
        if (p == NULL || (q != NULL && q < p))
        p = q;
      }

      if (p && p > file_name + 2 && p[1])
        {
          file_name = p + 1;

          while (file_name[0] && !SYS_IS_DIR_SEPARATOR (file_name[0]))
            file_name++;

          /* Possibly skip a backslash after the share name */
          if (SYS_IS_DIR_SEPARATOR (file_name[0]))
            file_name++;

          return (SysChar *)file_name;
        }
    }

  /* Skip initial slashes */
  if (SYS_IS_DIR_SEPARATOR (file_name[0])) {
    while (SYS_IS_DIR_SEPARATOR (file_name[0]))
      file_name++;
    return (SysChar *)file_name;
  }

  /* Skip X:\ */
  if (isalpha (file_name[0]) &&
      file_name[1] == ':' &&
      SYS_IS_DIR_SEPARATOR (file_name[2]))
    return (SysChar *)file_name + 3;

  return NULL;
}
