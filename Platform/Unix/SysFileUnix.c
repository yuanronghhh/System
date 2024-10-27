#include <System/Platform/Common/SysFile.h>
#include <System/Platform/Common/SysMem.h>

static SysChar exe_path[MAX_PATH];

FILE *sys_fopen(const SysChar *fpath, const SysChar * mode) {
  FILE *fp = fopen(fpath, mode);
  if (!fp){
    sys_warning_N("failed open file: %s",  fpath);
    return NULL;
  }

  return fp;
}

const SysChar *sys_exe_path(void) {
  SysInt bsize;
  SysInt nlen;
  SysChar *self = "/proc/self/exe";
  struct stat sb;
  SysChar *buf = &exe_path[0];

   if (lstat(self, &sb) == -1) {
       perror("lstat");
       exit(EXIT_FAILURE);
   }

   bsize = sb.st_size + 1;
   if (sb.st_size == 0) {
     bsize = MAX_PATH;
   }

   nlen = readlink(self, buf, bsize);

   if (nlen <= 0) {
     perror("get execute path failed");
     sys_free(buf);
     return NULL;
   }
   buf[nlen] = '\0';

   return buf;
}

void sys_fcloseall(void) {
  fcloseall();
}

SysInt sys_open(const SysChar *filename, SysInt flags, SysInt mode) {
  return open(filename, flags, mode);
}

SysSSize sys_read(SysInt fd, SysPointer buf, SysSize mcount) {
  return read(fd, buf, mcount);
}

SysInt sys_close(SysInt fd) {
  return close(fd);
}

SysBool sys_file_state_is_dir (SysFileState *state) {
  sys_return_val_if_fail(state != NULL, false);

  return state->is_dir;
}

/**
 * sys_file_read_link:
 *    read value of a symbolic link like readlink
 *
 * @filename: the symbolic link
 * @error: error message when failed
 *
 * Returns: newly-allocated path string
 */
SysChar *sys_file_read_link(const SysChar  *filename, SysError **error) {
  SysChar *buffer;
  size_t size;
  SysSize read_size;
  int saved_errno;

  sys_return_val_if_fail(filename != NULL, NULL);
  sys_return_val_if_fail (filename != NULL, NULL);
  sys_return_val_if_fail (error == NULL || *error == NULL, NULL);

  size = 256;
  buffer = sys_malloc (size);

  while (true) {
    read_size = readlink (filename, buffer, size);

    if (read_size < 0) {

      saved_errno = errno;
      if (error) {

        sys_error_set_N(error,
            filename,
            SYS_("Failed to read the symbolic link “%s”: %s"),
            saved_errno);
      }

      sys_free (buffer);
      return NULL;
    }

    if ((size_t) read_size < size) {

      buffer[read_size] = 0;
      return buffer;
    }

    size *= 2;
    buffer = sys_realloc (buffer, size);
  }

  sys_return_val_if_fail (filename != NULL, NULL);
  sys_return_val_if_fail (error == NULL || *error == NULL, NULL);

  sys_error_set_N (error,
      "%s",
      SYS_("Symbolic links not supported"));

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

SysBool sys_file_state_get_by_fp(FILE* fp, SysFileState* state) {
  sys_return_val_if_fail(state != NULL, false);

  SysInt fno;
  struct stat _fstate;

  if (fp == NULL) {
    printf("[Cannot Process NULL fp]\n");
    return false;
  }

  fno = fstat(fileno(fp), &_fstate);
  if (fno == -1) {
    return false;
  }

  state->st_size = _fstate.st_size;
  state->is_dir = S_ISDIR (_fstate.st_mode);

  return true;
}
