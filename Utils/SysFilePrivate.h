#ifndef __SYS_FILE_PRIVATE_H__
#define __SYS_FILE_PRIVATE_H__

#include <System/Utils/SysFile.h>

FILE *sys_real_fopen(const SysChar *fpath, const SysChar *mode);
SysSize sys_real_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp);

const SysChar *sys_real_exe_path(void);
SysInt sys_real_open(SysChar *filename, SysInt flags, SysInt mode);
void sys_real_fcloseall(void);

#endif
