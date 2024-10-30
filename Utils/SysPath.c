#include <System/Utils/SysPathPrivate.h>
#include <System/Utils/SysStr.h>

/**
 * sys_getcwd: get current workspace.
 *
 * Returns: new allocted string.
 */
SysChar *sys_getcwd(void) {
  return sys_real_getcwd();
}

/**
 * sys_path_escape: replace all windows '\\' to '/'
 * @buf:
 *
 * Returns: bool
 */
SysBool sys_path_escape(SysChar *buf) {
  // FIXME: better escape function
  return sys_str_override_c(buf, '\\', '/');
}

/**
 * sys_path_dirname: get dirname for path.
 * @path: path to parse.
 *
 * Returns: new allocated string.
 */
SysChar* sys_path_dirname(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, NULL);
  SysInt len = (SysInt)(sys_path_basename(path) - path);

  return sys_strndup(path, len - 1);
}

SysBool sys_path_exists(const SysChar *path) {
  return sys_real_path_exists(path);
}

/**
 * sys_path_name: return filename from path.
 * @path: path string.
 *
 * Returns: new allocated string for filename.
 */
SysChar *sys_path_name(const SysChar *path) {
  const SysChar *bname;
   SysChar *name;
   SysChar* p;

   bname = sys_path_basename(path);
   p = strrchr(bname, '.');

   name = sys_strndup(bname, p - bname);
   return name;
}

/**
 * sys_path_basename: get base name for path
 * @path: only path with '/' works
 *
 * Returns: const char path
 */
const SysChar *sys_path_basename(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, NULL);

  const SysChar *s = strrchr(path, '/');
  return s ? s + 1 : (SysChar *)path;
}

/**
 * sys_path_extension: get last extension name for path
 * @path: only path with '/' works
 *
 * Returns: return NULL if no dot match.
 */
const SysChar *sys_path_extension(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, NULL);

  const SysChar *s = strrchr(path, '.');
  return s ? s + 1 : (SysChar *)path;
}

/**
 * sys_path_purename: get name without last extension for path.
 * @path: path to parse.
 *
 * Returns: new allocated string.
 */
SysChar* sys_path_purename(const SysChar *path) {
  sys_return_val_if_fail(path != NULL, NULL);
  SysInt len = (SysInt)(sys_path_extension(path) - path);

  return sys_strndup(path, len - 1);
}

SysBool sys_path_is_absolute(const SysChar *path) {
  return sys_real_path_is_absolute(path);
}

