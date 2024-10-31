#ifndef __SYS_MAPPED_FILE_H__
#define __SYS_MAPPED_FILE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysMappedFile SysMappedFile;

SysMappedFile *sys_mapped_file_new (const SysChar  *filename,
    SysBool      writable,
    SysError      **error);
SysMappedFile *sys_mapped_file_new_from_fd  (SysInt          fd,
    SysBool      writable,
    SysError      **error);
SysSize        sys_mapped_file_get_length   (SysMappedFile  *file);
SysChar* sys_mapped_file_get_contents (SysMappedFile  *file);
SysBytes*     sys_mapped_file_get_bytes (SysMappedFile  *file);
SysMappedFile *sys_mapped_file_ref  (SysMappedFile  *file);
void         sys_mapped_file_unref  (SysMappedFile  *file);

SYS_END_DECLS

#endif
