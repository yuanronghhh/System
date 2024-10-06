#ifndef __SYS_ENUM_H__
#define __SYS_ENUM_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

const SysChar* sys_get_name_by_type(const SysChar *names[], SysInt len, SysInt type);
SysInt sys_get_type_by_name(const SysChar *names[], SysInt len, const SysChar *name);

SYS_END_DECLS

#endif
