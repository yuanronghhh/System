#ifndef __SYS_OS_H__
#define __SYS_OS_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

typedef enum _SYS_IO_ENUM {
  SYS_IO_INVALID,
  SYS_IO_PIPE,
  SYS_IO_STDOUT,
  SYS_IO_DEV_NULL
} SYS_IO_ENUM;

typedef struct _SysSArg SysSArg;

struct _SysSArg {
  int argc;
  SysChar** argv;
};

#define sys_sleep(ms) sys_usleep((unsigned long)ms)
#define SYS_(text) text
#define SYS_BACKTRACE_SIZE 100
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

SYS_API void sys_console_setup(void);
SYS_API bool sys_console_is_utf8(void);

SYS_API void sys_qsort_with_data(const SysPointer  pbase, SysInt total_elems,
  SysSize            size,
  SysCompareDataFunc compare_func,
  SysPointer         user_data);

SYS_API bool sys_set_env(const char *var, const char *value);
SYS_API const char *sys_get_env(const char *var);
SYS_API SysUInt64 sys_get_monotonic_time(void);
SYS_API void sys_usleep(unsigned long mseconds);
SYS_API void* sys_dlopen(const char *filename);
SYS_API void* sys_dlsymbol(void *handle, const char *symbol);
SYS_API void sys_dlclose(void* handle);
SYS_API SysUInt sys_nearest_pow(SysUInt num);
SYS_API SysDouble sys_rand_double(void);
SYS_API SysDouble sys_rand_double_range(SysDouble begin, SysDouble end);
SYS_API SysChar **sys_backtrace_string(SysInt *size);

SYS_API void sys_arg_init(SysSArg *self, SysInt argc, const SysChar* argv[]);
SYS_API int sys_arg_index(SysSArg *self, const SysChar *key, SysBool is_flag);

SYS_API void sys_setup(void);
SYS_API void sys_teardown(void);

SYS_END_DECLS

#endif
