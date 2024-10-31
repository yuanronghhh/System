#include <System/Utils/SysPathPrivate.h>
#include <System/Utils/SysStr.h>

/**
 * sys_getcwd: get current workspace.
 *
 * Returns: new allocted string.
 */
SysChar *sys_path_getcwd(void) {

  return sys_real_path_getcwd();
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

SysChar * sys_path_canonicalize_filename (const SysChar *filename,
                         const SysChar *relative_to)
{
  SysChar *canon, *input, *output, *after_root, *output_start;

  sys_return_val_if_fail (relative_to == NULL || sys_path_is_absolute (relative_to), NULL);

  if (!sys_path_is_absolute (filename))
    {
      SysChar *cwd_allocated = NULL;
      const SysChar  *cwd;

      if (relative_to != NULL)
        cwd = relative_to;
      else
        cwd = cwd_allocated = sys_getcwd();

      canon = sys_path_build_filename (cwd, filename, NULL);
      sys_free (cwd_allocated);
    }
  else
    {
      canon = sys_strdup (filename);
    }

  after_root = (char *)sys_path_skip_root (canon);

  if (after_root == NULL)
    {
      /* This shouldn't really happen, as sys_get_current_dir() should
         return an absolute pathname, but bug 573843 shows this is
         not always happening */
      sys_free (canon);

      return sys_path_build_filename (SYS_DIR_SEPARATOR, filename, NULL);
    }

  /* Find the first dir separator and use the canonical dir separator. */
  for (output = after_root - 1;
       (output >= canon) && SYS_IS_DIR_SEPARATOR (*output);
       output--)
    *output = SYS_DIR_SEPARATOR;

  /* 1 to re-increment after the final decrement above (so that output >= canon),
   * and 1 to skip the first `/`. There might not be a first `/` if
   * the @canon is a Windows `//server/share` style path with no
   * trailing directories. @after_root will be '\0' in that case. */
  output++;
  if (*output == SYS_DIR_SEPARATOR)
    output++;

  /* POSIX allows double slashes at the start to mean something special
   * (as does windows too). So, "//" != "/", but more than two slashes
   * is treated as "/".
   */
  if (after_root - output == 1)
    output++;

  input = after_root;
  output_start = output;
  while (*input)
    {
      /* input points to the next non-separator to be processed. */
      /* output points to the next location to write to. */
      sys_assert (input > canon && SYS_IS_DIR_SEPARATOR (input[-1]));
      sys_assert (output > canon && SYS_IS_DIR_SEPARATOR (output[-1]));
      sys_assert (input >= output);

      /* Ignore repeated dir separators. */
      while (SYS_IS_DIR_SEPARATOR (input[0]))
       input++;

      /* Ignore single dot directory components. */
      if (input[0] == '.' && (input[1] == 0 || SYS_IS_DIR_SEPARATOR (input[1])))
        {
           if (input[1] == 0)
             break;
           input += 2;
        }
      /* Remove double-dot directory components along with the preceding
       * path component. */
      else if (input[0] == '.' && input[1] == '.' &&
               (input[2] == 0 || SYS_IS_DIR_SEPARATOR (input[2])))
        {
          if (output > output_start)
            {
              do
                {
                  output--;
                }
              while (!SYS_IS_DIR_SEPARATOR (output[-1]) && output > output_start);
            }
          if (input[2] == 0)
            break;
          input += 3;
        }
      /* Copy the input to the output until the next separator,
       * while converting it to canonical separator */
      else
        {
          while (*input && !SYS_IS_DIR_SEPARATOR (*input))
            *output++ = *input++;
          if (input[0] == 0)
            break;
          input++;
          *output++ = SYS_DIR_SEPARATOR;
        }
    }

  /* Remove a potentially trailing dir separator */
  if (output > output_start && SYS_IS_DIR_SEPARATOR (output[-1]))
    output--;

  *output = '\0';

  return canon;
}

