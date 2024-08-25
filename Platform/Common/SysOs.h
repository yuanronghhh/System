#ifndef __SYS_OS_H__
#define __SYS_OS_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

struct _SysElapse {
  SysUInt64 start;
  SysUInt64 end;
  SysUInt64 diff;
  const SysChar *name;
};

struct _SysSArg {
  SysInt argc;
  SysChar** argv;
};

#define sys_sleep(ms) sys_usleep((unsigned long)ms)
#define SYS_(text) text
#define SYS_BACKTRACE_SIZE 100
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

SYS_API void sys_console_setup(void);
SYS_API SysBool sys_console_is_utf8(void);

SYS_API void sys_qsort_with_data(const SysPointer  pbase, SysInt total_elems,
  SysSize            size,
  SysCompareDataFunc compare_func,
  SysPointer         user_data);

SYS_API SysBool sys_set_env(const SysChar *var, const SysChar *value);
SYS_API const SysChar *sys_get_env(const SysChar *var);
SYS_API SysUInt64 sys_get_monotonic_time(void);
SYS_API void sys_usleep(unsigned long mseconds);
SYS_API void* sys_dlopen(const SysChar *filename);
SYS_API void* sys_dlmodule(const SysChar *name);
SYS_API void* sys_dlsymbol(void *handle, const SysChar *symbol);
SYS_API void sys_dlclose(void* handle);
SYS_API SysUInt sys_nearest_pow(SysUInt num);
SYS_API SysDouble sys_rand_double(void);
SYS_API SysDouble sys_rand_double_range(SysDouble begin, SysDouble end);
SYS_API SysChar **sys_backtrace_string(SysInt *size);

SYS_API void sys_arg_init(SysSArg *self, SysInt argc, const SysChar* argv[]);
SYS_API SysInt sys_arg_index(SysSArg *self, const SysChar *key, SysBool is_flag);

SYS_API void sys_elapse_begin(SysElapse *self, const SysChar *name);
SYS_API void sys_elapse_end(SysElapse *self);

SYS_API void sys_set_debugger (SysBool bvalue);
SYS_API SysBool sys_get_debugger (void);

SYS_END_DECLS

#endif
