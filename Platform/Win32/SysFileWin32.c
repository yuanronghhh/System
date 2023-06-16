#include <System/Utils/SysPath.h>
#include <System/Utils/SysFilePrivate.h>

static SysChar exepath[MAX_PATH];

FILE *sys_real_fopen(const SysChar *filename, const SysChar * mode) {
  FILE *fp;

  errno_t eno = fopen_s(&fp, filename, mode);
  if (eno == 0) {
    return fp;
  }

  sys_error_N("%s: %s", sys_strerr(errno), filename);
  return NULL;
}

SysSize sys_real_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp) {
  return fread_s(buffer, bufsize, elem_size, elem_count, fp);
}

SysInt sys_real_open(SysChar *filename, SysInt flags, SysInt mode) {
  SysInt fp;
  SysInt shareflag = SH_DENYNO;

  errno_t eno = _sopen_s(&fp, filename, flags, shareflag, mode);
  if (eno == 0) {
    return fp;
  }

  sys_error_N("%s: %s", sys_strerr(errno), filename);
  return -1;
}

const SysChar *sys_real_exe_path(void) {
  int nlen;
  SysChar *sp;

  sp = &exepath[0];

  nlen = GetModuleFileName(0, sp, MAX_PATH);
  sys_path_escape(sp);

  return sp;
}

void sys_real_fcloseall(void) {
  _fcloseall();
}
