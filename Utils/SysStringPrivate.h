#ifndef __SYS_STRING_PRIVATE_H__
#define __SYS_STRING_PRIVATE_H__

#include <System/Utils/SysString.h>

SYS_BEGIN_DECLS

int sys_real_vsprintf(SysChar* str, SysSize size, const SysChar* format, va_list args);
SYS_END_DECLS

#endif
