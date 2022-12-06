#ifndef __SYS_OS_H__
#define __SYS_OS_H__

#include <Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

typedef enum _SYS_IO_TYPE {
  SYS_IO_INVALID,
  SYS_IO_PIPE,
  SYS_IO_STDOUT,
  SYS_IO_DEV_NULL
} SYS_IO_TYPE;

#define sys_sleep(ms) sys_usleep((unsigned long)ms)
#define SYS_(text) text

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


SYS_API void sys_init_console(void);
SYS_API bool sys_console_is_utf8(void);

SYS_API bool sys_set_env(const char *var, const char *value);
SYS_API const char *sys_get_env(const char *var);
SYS_API SysUInt64 sys_get_monoic_time(void);
SYS_API void sys_usleep(unsigned long mseconds);
SYS_API void* sys_dlopen(const char *filename);
SYS_API void* sys_dlsymbol(void *handle, const char *symbol);
SYS_API void sys_dlclose(void* handle);
SYS_API SysUInt sys_nearest_pow(SysUInt num);
SYS_API void sys_init(void);

SYS_END_DECLS

#endif
