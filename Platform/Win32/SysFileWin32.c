#include <System/Utils/SysFile.h>
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
  int nlen;
  SysChar *sp;

  sp = &exepath[0];

  nlen = GetModuleFileName(0, sp, MAX_PATH);
  sys_path_escape(sp);

  return sp;
}

void sys_fcloseall(void) {
  _fcloseall();
}

SysBool sys_file_get_contents (const SysChar *filename,
                     SysChar       **contents,
                     SysSize       *length,
                     SysError      **error) {

  sys_return_val_if_fail (filename != NULL, false);
  sys_return_val_if_fail (length != NULL, false);

  struct stat st;
  int fd;
  SysSize offset;
  SysSSize nread;
  SysChar *content = NULL;

  fd = sys_open(filename, O_RDONLY, S_IREAD);
  if(fd == -1) {
    sys_error_set_N(error, "open file failed: %s", filename);
    goto fail;
  }

  if (fstat(fd, &st) != 0) {
    sys_error_set_N(error, "fstate failed: %s", filename);
    goto fail;
  }

  content = sys_malloc_N(st.st_size + 1);
  offset = 0;
  do {
    nread = sys_read(fd, content + offset, st.st_size - offset);

    if (nread < 0) {
      goto fail;
    }
    
    if (nread > 0) {
      offset += nread;
    }
  } while ((nread > 0 && offset < st.st_size) || (nread == -1 && errno == EINTR));

#if SYS_OS_WIN32
  *length = offset;
  content[offset] = '\0';
#else
  *length = st.st_size;
  content[st.st_size] = '\0';
#endif

  *contents = content;
  return true;
fail:
  if(content != NULL) {
    sys_free_N(content);
  }

  sys_close(fd);
  return false;
}
