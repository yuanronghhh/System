#ifndef __SYS_OS_PRIVATE_H__
#define __SYS_OS_PRIVATE_H__

#include <System/Platform/Common/SysOs.h>

void sys_real_init_console(void);
SysBool sys_real_console_is_utf8(void);

void sys_real_setup(void);
void sys_real_teardown(void);

SysInt sys_real_poll (SysPollFD *fds,
    SysUInt nfds,
    SysInt flags,
    SysInt timeout);
const SysChar *sys_real_env_get(const SysChar *var);
SysBool sys_real_env_set(const SysChar *var, const SysChar *value);
SysUInt64 sys_real_get_monotonic_time(void);
void sys_real_usleep(unsigned long mseconds);
SysPointer sys_real_dlopen(const SysChar *filename);
SysPointer sys_real_dlmodule(const SysChar *name);
SysPointer sys_real_dlsymbol(void *handle, const SysChar *symbol);
void sys_real_dlclose(void* handle);
SysChar **sys_real_backtrace_string(SysInt *size);

#endif
