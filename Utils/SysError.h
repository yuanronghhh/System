#ifndef __SYS_ERROR_H__
#define __SYS_ERROR_H__

#include <System/Fundamental/SysCommon.h>

#define sys_assert(expr) assert(expr)

#define sys_debug_N(format, ...) sys_log(SYS_LOG_ARGS(sys_debug, format) stdout, SYS_LOG_DEBUG, format, __VA_ARGS__)
#define sys_info_N(format, ...) sys_log(SYS_LOG_ARGS(sys_info, format) stdout, SYS_LOG_INFO, format, __VA_ARGS__)
#define sys_warning_N(format, ...) sys_warning(SYS_LOG_ARGS(sys_warning, format) format, __VA_ARGS__)
#define sys_abort_N(format, ...) sys_abort(SYS_LOG_ARGS(sys_error, format) format, __VA_ARGS__)
#define sys_exit_N(exitcode, format, ...) sys_exit(SYS_LOG_ARGS(sys_error, exitcode) exitcode, format, __VA_ARGS__)
#define sys_error_set_N(error, format, ...) sys_error_set(SYS_LOG_ARGS(sys_error_set, format) error, format, __VA_ARGS__)

#define sys_error_N(format, ...) \
do {                                                          \
  sys_error(SYS_LOG_ARGS(sys_error, format) format, __VA_ARGS__); \
} while(0)

#define sys_return_if_fail(expr)              \
do                                            \
{                                             \
  if (!(expr)) {                              \
    sys_warning_N("%s %s.", #expr, "failed"); \
    return;                                   \
  }                                           \
} while(0)

#define sys_return_val_if_fail(expr, ret)    \
do {                                         \
  if (!(expr)) {                             \
    sys_warning_N("%s %s.", #expr,"failed"); \
    return ret;                              \
  }                                          \
} while(0)

typedef struct _SysError SysError;

typedef enum _SYS_LOG_LEVEL {
  SYS_LOG_RESET,
  SYS_LOG_DEBUG,
  SYS_LOG_INFO,
  SYS_LOG_WARNING,
  SYS_LOG_ERROR,
  SYS_LOG_RECORD
} SYS_LOG_LEVEL;

struct _SysError {
  SysChar *message;

  const SysChar *func;
  SysInt line;
};

SYS_API void sys_use_debugger (SysBool value);
SYS_API void sys_break (void);
SYS_API void sys_abort (SYS_LOG_ARGS_N const SysChar* format, ...);
SYS_API void sys_exit (SYS_LOG_ARGS_N SysInt exitcode, const SysChar* format, ...);
SYS_API void sys_warning (SYS_LOG_ARGS_N const SysChar* format, ...);
SYS_API void sys_log (SYS_LOG_ARGS_N FILE* std, SYS_LOG_LEVEL level, const SysChar* format, ...);
SYS_API void sys_error (SYS_LOG_ARGS_N const SysChar* format, ...);
SYS_API void sys_verror (SYS_LOG_ARGS_N const SysChar* format, va_list args);
SYS_API SysError* sys_error_new (void);
SYS_API void sys_error_free (SysError* err);
SYS_API void sys_error_set (SYS_LOG_ARGS_N SysError** err, const SysChar* format, ...);
SYS_API void sys_vlog(SYS_LOG_ARGS_N FILE* std, SYS_LOG_LEVEL level, const SysChar* format, va_list args);
SYS_API const SysChar* sys_strerror(SysInt errnum);

void sys_error_setup(void);
void sys_error_teardown(void);

#endif
