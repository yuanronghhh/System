#include <System/Platform/Common/SysMem.h>
#include <System/Utils/SysFile.h>

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
