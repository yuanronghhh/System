#ifndef __SYS_TIO_H__
#define __SYS_TIO_H__

#include <System/DataTypes/SysHashTable.h>

SYS_BEGIN_DECLS

typedef struct _SysTextIO SysTextIO;

SYS_API SysTextIO *sys_tio_new(const SysChar *filename);
SYS_API void sys_tio_free(SysTextIO *cg);
SysChar *sys_tio_filename(SysTextIO *cg);
SYS_API void sys_tio_set_src(SysTextIO *cg, const SysChar *srcname);
SYS_API void sys_tio_write(SysTextIO *cg, const SysChar *str);
SYS_API void sys_tio_write_source(SysTextIO *cg,
    SysInt src_line, const SysChar *content);
SYS_API bool sys_tio_write_template(SysTextIO *cg, SysChar *tpl, SysHashTable *map);


SYS_END_DECLS

#endif
