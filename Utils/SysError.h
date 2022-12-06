#ifndef __SYS_ERROR_H__
#define __SYS_ERROR_H__

#include <Fundamental/SysCommon.h>

#define sys_assert(expr) assert(expr)

#define sys_debug_N(format, ...) sys_log(SYS_ARGS_FIXED(sys_debug, format) stdout, SYS_LOG_DEBUG, format, __VA_ARGS__)
#define sys_info_N(format, ...) sys_log(SYS_ARGS_FIXED(sys_info, format) stdout, SYS_LOG_INFO, format, __VA_ARGS__)
#define sys_warning_N(format, ...) sys_warning(SYS_ARGS_FIXED(sys_warning, format) format, __VA_ARGS__)
#define sys_abort_N(format, ...) sys_abort(SYS_ARGS_FIXED(sys_error, format) format, __VA_ARGS__)
#define sys_error_N(format, ...) sys_error(SYS_ARGS_FIXED(sys_error, format) format, __VA_ARGS__)
#define sys_exit_N(exitcode, format, ...) sys_exit(SYS_ARGS_FIXED(sys_error, exitcode) exitcode, format, __VA_ARGS__)
#define sys_error_set_N(error, format, ...) sys_error_set(SYS_ARGS_FIXED(sys_error_set, format) error, format, __VA_ARGS__)

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

#define sys_error_E(expr, msg) do {       \
  if (!(expr)) {                          \
    sys_error_N("%s", msg);               \
  }                                       \
} while(0)

#define sys_abort_E(expr, msg) do {       \
  if (!(expr)) {                          \
    sys_abort_N("%s:%s", sys_strerr(errno), msg); \
  }                                       \
} while(0)

#define SYS_SUCCESS 0
#define SYS_FAILED 1

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

SYS_API  void       sys_break          (void);
SYS_API  void       sys_abort          (SYS_ARGS_N_FIXED  const      SysChar*    format,        ...);
SYS_API  void       sys_exit           (SYS_ARGS_N_FIXED  SysInt     exitcode,   const          SysChar*   format,     ...);
SYS_API  void       sys_warning        (SYS_ARGS_N_FIXED  const      SysChar*    format,        ...);
SYS_API  void       sys_log            (SYS_ARGS_N_FIXED  FILE*      std,        SYS_LOG_LEVEL  level,     const       SysChar*   format,  ...);
SYS_API  void       sys_error          (SYS_ARGS_N_FIXED  const      SysChar*    format,        ...);
SYS_API  void       sys_verror         (SYS_ARGS_N_FIXED  const      SysChar*    format,        va_list    args);
SYS_API  SysError*  sys_error_new      (void);
SYS_API  void       sys_error_free     (SysError*         err);
SYS_API  void       sys_error_set      (SYS_ARGS_N_FIXED  SysError*  *err,       const          SysChar*   format,     ...);
SYS_API  const SysChar*   sys_strerr(SysInt  errnum);

#endif
