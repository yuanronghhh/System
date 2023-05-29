#ifndef __SYS_FILE_H__
#define __SYS_FILE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysFileState SysFileState;

struct _SysFileState {
  SysSize st_size;
};

#define sys_fappend(fp, line) sys_fwrite_string(fp, line, sys_strlen(s, MAX_LINE))

SYS_API SysChar* sys_freadline(SysChar** s, SysSize* len, FILE* fp);
SYS_API SysSize sys_fwrite(const void* buf, SysSize size, SysSize n, FILE* fp);
SYS_API SysSize sys_fwrite_string(FILE* fp, const SysChar* s, SysSize len);
SYS_API SysSize sys_vfformat(FILE* fp, SysChar** maxbuf, const SysChar* format, va_list args);

SYS_API SysInt sys_vfprintf(FILE* const fp, SysChar const* const format, va_list args);
SYS_API SysInt sys_fprintf(FILE* const fp, SysChar  const* const format, ...);
SYS_API SysInt sys_open(SysChar* filename, SysInt flags, SysInt mode);
SYS_API SysInt sys_fputs(FILE* const fp, SysChar const* s);
SYS_API SysChar* sys_fgets(SysChar* buf, SysInt max, FILE* fp);
SYS_API SysInt sys_putc(SysInt c, FILE* fp);
SYS_API SysInt sys_fgetc(FILE* fp);
SYS_API SysInt sys_ungetc(SysInt c, FILE* fp);
SYS_API FILE* sys_fopen(const SysChar* filename, const SysChar* mode);
SYS_API SysSize sys_fread(void*  buffer,SysSize bufsize,SysSize elem_size,SysSize elem_count, FILE*  fp);
SYS_API void sys_fclose(FILE* fp);
SYS_API void sys_fcloseall(void);
SYS_API bool sys_fstat(FILE*  fp, SysFileState* state);
SYS_API const SysChar* sys_exe_path(void);

SYS_END_DECLS

#endif
