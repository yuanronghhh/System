#include <System/Platform/Common/SysFile.h>
#include <System/Utils/SysPath.h>

static SysChar exepath[MAX_PATH];

FILE *sys_fopen(const SysChar *filename, const SysChar * mode) {
  FILE *fp;

  errno_t eno = fopen_s(&fp, filename, mode);
  if (eno == 0) {
    return fp;
  }

  sys_error_N("%s: %s", sys_strerror(errno), filename);
  return NULL;
}

SysSize sys_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp) {
  return fread_s(buffer, bufsize, elem_size, elem_count, fp);
}

SysInt sys_open(const SysChar *filename, SysInt flags, SysInt mode) {
  SysInt fp;
  SysInt shareflag = SH_DENYNO;

  errno_t eno = _sopen_s(&fp, filename, flags, shareflag, mode);
  if (eno == 0) {
    return fp;
  }

  sys_error_N("%s: %s", sys_strerror(errno), filename);
  return -1;
}

SysInt sys_close(SysInt fd) {
  return _close(fd);
}

SysSSize sys_read(SysInt fd, SysPointer buf, SysSize mcount) {
  return _read(fd, buf, (SysUInt)mcount);
}

const SysChar *sys_exe_path(void) {
  SysULong nlen;
  SysChar *sp;

  sp = &exepath[0];

  nlen = GetModuleFileName(0, sp, MAX_PATH);
  sys_path_escape(sp);
  UNUSED(nlen);

  return sp;
}

void sys_fcloseall(void) {
  _fcloseall();
}

SysChar *sys_file_read_link(const SysChar  *filename, SysError **error) {
  sys_return_val_if_fail(filename == NULL, NULL);

  sys_warning_N("%s", "not implements");

  return NULL;
}

SysBool sys_file_state_get_by_filename(const SysChar *filename, SysFileState* state) {
  SysInt fno;
  struct stat _fstate;

  sys_return_val_if_fail(state != NULL, false);
  sys_return_val_if_fail(filename != NULL, false);

  fno = stat(filename, &_fstate);
  if (fno == -1) { return false; }

  state->st_size = _fstate.st_size;
  state->is_dir = S_ISDIR (_fstate.st_mode);

  return true;
}

static int
_g_win32_stat_utf8 (const SysChar       *filename,
                    struct stat *buf,
                    SysBool for_symlink)
{
  wchar_t *wfilename;
  int result;
  SysSize len;

  if (filename == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  len = strlen (filename);

  while (len > 0 && SYS_IS_DIR_SEPARATOR (filename[len - 1]))
    len--;

  if (len <= 0 ||
      (sys_path_is_absolute (filename) && len <= (SysSize) (g_path_skip_root (filename) - filename)))
    len = strlen (filename);

  wfilename = g_utf8_to_utf16 (filename, len, NULL, NULL, NULL);

  if (wfilename == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  result = _g_win32_stat_utf16_no_trailing_slashes (wfilename, buf, for_symlink);

  g_free (wfilename);

  return result;
}

int sys_win32_lstat_utf8 (const SysChar       *filename,
                    struct stat *buf)
{
  return _g_win32_stat_utf8 (filename, buf, TRUE);
}

SysInt sys_lstat(const SysChar *filename, struct stat * state) {
  sys_return_val_if_fail(state != NULL, -1);
  sys_return_val_if_fail(filename != NULL, -1);

  return sys_win32_lstat_utf8 (filename, state);
}

