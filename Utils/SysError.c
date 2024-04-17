#include <System/Utils/SysString.h>
#include <System/Utils/SysFile.h>
#include <System/Utils/SysErrorPrivate.h>

static SysChar* errColors[] = {
  "\033[0m",
  "\033[1;37m",
  "\033[1;37m",
  "\033[1;33m",
  "\033[1;31m",
  "\033[1;35m",
};

static SYS_INLINE SysChar* get_color(SYS_LOG_LEVEL level) {
  return errColors[level];
}

void sys_break(void) {

#if SYS_DEBUG
  #if SYS_OS_WIN32
    DebugBreak();
  #endif

  #if SYS_OS_UNIX
    // __asm__ __volatile__("int $03");
    raise(SIGTRAP);
  #endif
#endif
}

void sys_vlog(SYS_LOG_ARGS_N FILE* std, SYS_LOG_LEVEL level, const SysChar* format, va_list args) {
  SYS_LEAK_IGNORE_BEGIN;
  sys_fprintf(std, "%s[%s:%d] ", get_color(level), _funcname, _line);
  sys_vfprintf(std, format, args);
  sys_fprintf(std, "%s\n", get_color(SYS_LOG_RESET));
  SYS_LEAK_IGNORE_END;
}

void sys_log(SYS_LOG_ARGS_N FILE* std, SYS_LOG_LEVEL level, const SysChar* format, ...) {
  va_list args;
  va_start(args, format);
  sys_vlog(SYS_LOG_ARGS_P std, level, format, args);
  va_end(args);
}

void sys_warning(SYS_LOG_ARGS_N const SysChar* format, ...) {
  va_list args;
  va_start(args, format);
  sys_vlog(SYS_LOG_ARGS_P stderr, SYS_LOG_WARNING, format, args);
  va_end(args);

#if SYS_DEBUG
  const SysChar* err = sys_strerror(errno);
  UNUSED(err);

  sys_break();
#endif
}

void sys_abort(SYS_LOG_ARGS_N const SysChar* format, ...) {
  va_list args;
  va_start(args, format);

  sys_verror(SYS_LOG_ARGS_P format, args);
  va_end(args);

  abort();
}

void sys_exit(SYS_LOG_ARGS_N SysInt exitcode, const SysChar* format, ...) {
  va_list args;
  va_start(args, format);

  sys_verror(SYS_LOG_ARGS_P format, args);
  va_end(args);

  exit(exitcode);
}

void sys_error(SYS_LOG_ARGS_N const SysChar* format, ...) {
  va_list args;
  va_start(args, format);

  sys_verror(SYS_LOG_ARGS_P format, args);

  va_end(args);
}

void sys_verror(SYS_LOG_ARGS_N const SysChar* format, va_list args) {
  sys_vlog(SYS_LOG_ARGS_P stderr, SYS_LOG_ERROR, format, args);

#if SYS_DEBUG
  const SysChar *err = sys_strerror(errno);
  UNUSED(err);

  sys_break();
#endif
}

SysError* sys_error_new(void) {
  SysError* nerror = sys_new0_N(SysError, 1);

  nerror->message = NULL;

  return nerror;
}

void sys_error_set(SYS_LOG_ARGS_N SysError**err, const SysChar* format, ...) {
  sys_return_if_fail(err != NULL);
  sys_return_if_fail(*err == NULL);

  SysError* nerror = NULL;
  if ((*err) != NULL) {
    sys_warning_N("Error was set but bot handle: %s\n", (*err)->message);
    sys_free_N((*err)->message);
    nerror = (*err);
  } else {
    nerror = sys_error_new();
    *err = nerror;
  }

  nerror->func = _funcname;
  nerror->line = _line;

  va_list args;
  va_start(args, format);

  sys_vasprintf(&nerror->message, format, args);

  va_end(args);
}

void sys_error_free(SysError* err) {
  sys_return_if_fail(err != NULL);

  if (err->message != NULL) {
    sys_free_N(err->message);
  }

  sys_free_N(err);
}

/**
 * sys_strerror: get recognized string from errno.
 * @errnum: errno
 *
 * Returns: void
 */
const SysChar* sys_strerror(SysInt errnum) {
  return sys_real_strerr(errnum);
}
