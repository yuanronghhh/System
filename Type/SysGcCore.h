#ifndef __SYS_MS_CORE_H__
#define __SYS_MS_CORE_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#if USE_MARKSWEEP
#include <System/Type/MarkSweep/SysMarkSweep.h>
#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMsDwarf.h>
#endif

void sys_gc_setup(void);
void sys_gc_teardown(void);

SYS_END_DECLS

#endif
