#include <Utils/SysString.h>
#include <Utils/SysFile.h>
#include <Utils/SysErrorPrivate.h>

static SysChar* errColors[] = {
  "\033[0m",
  "\033[1;30m",
  "\033[1;34m",
  "\033[1;33m",
  "\033[1;31m",
  "\033[1;35m",
};

static bool record_state = false;
static const SysChar* record_only_func = NULL;
static const SysChar* record_after_func = NULL;

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
    // raise(SIGTRAP);
  #endif
#endif
}

static void sys_vlog(SYS_ARGS_N_FIXED FILE* std, SYS_LOG_LEVEL level, const SysChar* format, va_list args) {
  sys_fprintf(std, "%s[%s:%d] ", get_color(level), _funcname, _line);
  sys_vfprintf(std, format, args);
  sys_fprintf(std, "%s\n", get_color(SYS_LOG_RESET));
}

void sys_log(SYS_ARGS_N_FIXED FILE* std, SYS_LOG_LEVEL level, const SysChar* format, ...) {
  va_list args;
  va_start(args, format);
  sys_vlog(SYS_ARGS_P_FIXED(sys_vlog, format) std, level, format, args);
  va_end(args);
}

void sys_warning(SYS_ARGS_N_FIXED const SysChar* format, ...) {
  va_list args;
  va_start(args, format);
  sys_vlog(SYS_ARGS_P_FIXED(sys_vlog, format) stderr, SYS_LOG_WARNING, format, args);
  va_end(args);

#if SYS_DEBUG
  sys_break(); 
#endif
}

void sys_record(SYS_ARGS_N_FIXED const SysChar* unused) {
  UNUSED(unused);

  if (record_only_func) {
    record_state = sys_streq(_funcname, record_only_func) ? true : false;
  } else if (record_after_func) {
    if (!record_state) {
      if (sys_streq(_funcname, record_after_func)) {
        record_state = true;
      }
    }
  } else {
    return;
  }

  if (!record_state) {
    return;
  }

  sys_fprintf(stdout, "%s[Record] %s\t%d\t%s\t%s\t%s\n",
      get_color(SYS_LOG_RECORD),
      _funcname, _line, _callfunc, _ptrstr,
      get_color(SYS_LOG_RESET));
}

void sys_abort(SYS_ARGS_N_FIXED const SysChar* format, ...) {
  va_list args;
  va_start(args, format);

  sys_verror(SYS_ARGS_P_FIXED(sys_error, format) format, args);
  va_end(args);

  abort();
}

void sys_exit(SYS_ARGS_N_FIXED SysInt exitcode, const SysChar* format, ...) {
  va_list args;
  va_start(args, format);

  sys_verror(SYS_ARGS_P_FIXED(sys_error, format) format, args);
  va_end(args);

  exit(exitcode);
}

void sys_error(SYS_ARGS_N_FIXED const SysChar* format, ...) {
  va_list args;
  va_start(args, format);

  sys_verror(SYS_ARGS_P_FIXED(sys_error, format) format, args);

  va_end(args);
}

void sys_verror(SYS_ARGS_N_FIXED const SysChar* format, va_list args) {
  sys_vlog(SYS_ARGS_P_FIXED(sys_log, msg) stderr, SYS_LOG_ERROR, format, args);

#if SYS_DEBUG
  const SysChar *err = sys_strerr(errno);
  sys_break();
#endif
}

SysError* sys_error_new(void) {
  SysError* nerror = sys_new0_N(SysError, 1);

  nerror->message = NULL;

  return nerror;
}

void sys_error_set(SYS_ARGS_N_FIXED SysError* *err, const SysChar* format, ...) {
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
 * sys_strerr: get recognized string from errno.
 * @errnum: errno
 *
 * Returns: void
 */
const SysChar* sys_strerr(SysInt errnum) {
  return sys_real_strerr(errnum);
}
