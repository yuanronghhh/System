#ifndef __SYS_REF_GC_H__
#define __SYS_REF_GC_H__

#include <System/Type/Ref/SysRefBlock.h>
#include <System/Type/SysType.h>

SYS_BEGIN_DECLS

SYS_API void sys_gc_setup(void);
SYS_API void sys_gc_teardown(void);

SYS_END_DECLS

#endif
