#ifndef __SYS_QUARK_H__
#define __SYS_QUARK_H__

#include <System/DataTypes/SysHashTable.h>

SYS_API const SysChar *sys_quark_string(const SysChar *msg);
SYS_API void sys_quark_setup(void);
SYS_API void sys_quark_teardown(void);

#endif
