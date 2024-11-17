#include <System/Utils/SysErrorPrivate.h>
#include <System/Utils/SysStr.h>
#include <System/Platform/Common/SysFile.h>
#include <System/Platform/Common/SysThread.h>
#include <System/DataTypes/SysQuark.h>

static SysChar* errColors[] = {
  "\033[0m",
  "\033[1;37m",
  "\033[1;37m",
  "\033[1;33m",
  "\033[1;31m",
  "\033[1;35m",
};

#if defined(SYS_OS_ANDROID)
static SysChar g_android_tag[255] = {0};
static SysInt androidPrior[] = {
  ANDROID_LOG_UNKNOWN,
  ANDROID_LOG_DEBUG,
  ANDROID_LOG_INFO,
  ANDROID_LOG_WARN,
  ANDROID_LOG_ERROR,
  ANDROID_LOG_FATAL,
  ANDROID_LOG_SILENT
};
#endif

static SysMutex g_log_lock;

static SYS_INLINE SysChar* get_color(SYS_LOG_LEVEL level) {
  return errColors[level];
}

#if defined(SYS_OS_ANDROID)
void sys_set_android_log_tag(const SysChar *tag) {
  SysSize len = strlen(tag);
  sys_return_if_fail(len <= 255);

  sys_strcpy(g_android_tag, tag);
}
#endif

void sys_break(void) {
  if(!sys_get_debugger()) {
    return;
  }

#if defined(SYS_OS_WIN32)
  if (IsDebuggerPresent ()) {
    DebugBreak();
  }
#elif defined(SYS_OS_UNIX)
  // __asm__ __volatile__("int $03");
  raise(SIGTRAP);
#endif
}

static SysInt sys_log_fprintf(FILE* std, SYS_LOG_LEVEL level, const SysChar *format, ...) {
  SysInt len;

  va_list args;
  va_start(args, format);

#if defined(SYS_OS_ANDROID)
  len = __android_log_vprint(androidPrior[level], &g_android_tag[0], format, args);
#else
  len = sys_vfprintf(std, format, args);
#endif

  va_end(args);

  return len;
}

static SysInt sys_log_vfprintf(FILE* std, SYS_LOG_LEVEL level, const SysChar* format, va_list args) {
  SysInt len;

#if defined(SYS_OS_ANDROID)
  len = __android_log_vprint(androidPrior[level], &g_android_tag[0], format, args);
#else
  len = sys_vfprintf(std, format, args);
#endif

  return len;
}

void sys_vlog(SYS_LOG_ARGS_N FILE* std, SYS_LOG_LEVEL level, const SysChar* format, va_list args) {
  sys_mutex_lock(&g_log_lock);
  SYS_LEAK_IGNORE_BEGIN;

  sys_log_fprintf(std, level, "%s[%s:%d] ", get_color(level), _funcname, _line);
  sys_log_vfprintf(std, level, format, args);
  sys_log_fprintf(std, level, "%s\n", get_color(SYS_LOG_RESET));

  SYS_LEAK_IGNORE_END;
  sys_mutex_unlock(&g_log_lock);
}

void sys_log(SYS_LOG_ARGS_N FILE* std, SYS_LOG_LEVEL level, const SysChar* format, ...) {
  va_list args;
  va_start(args, format);
#if defined(SYS_OS_ANDROID)
  __android_log_vprint(androidPrior[level], &g_android_tag[0], format, args);
#else
  sys_vlog(SYS_LOG_ARGS_P std, level, format, args);
#endif
  va_end(args);
}

void sys_warning(SYS_LOG_ARGS_N const SysChar* format, ...) {
  va_list args;
  va_start(args, format);
  sys_vlog(SYS_LOG_ARGS_P stderr, SYS_LOG_WARNING, format, args);
  va_end(args);

  sys_break();
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

  sys_break();
}

SysError* sys_error_new(void) {
  SysError* nerror = sys_new0(SysError, 1);

  nerror->message = NULL;

  return nerror;
}

void sys_error_set(SYS_LOG_ARGS_N SysError**err, const SysChar* format, ...) {
  sys_return_if_fail(err != NULL);
  sys_return_if_fail(*err == NULL);

  SysError* nerror = NULL;
  if ((*err) != NULL) {
    sys_warning_N("Error was set but not handle: %s\n", (*err)->message);
    sys_free((*err)->message);
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

void sys_error_propagate (SysError **dest,
                   SysError  *src) {
  sys_return_if_fail (src != NULL);

  if (dest == NULL) {

    sys_error_free (src);
    return;

  } else {

    if (*dest != NULL) {

      sys_warning_N ("%s", src->message);
      sys_error_free (src);

    } else {

      *dest = src;
    }
  }
}

void sys_error_free(SysError* err) {
  sys_return_if_fail(err != NULL);

  if (err->message != NULL) {
    sys_free(err->message);
  }

  sys_free(err);
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

SysChar *sys_error_message(SysInt err) {

  return sys_error_real_message(err);
}

SysBool sys_error_matches (const SysError *error,
                 SysQuark        domain,
                 SysInt          code) {
  return error &&
    error->domain == domain &&
    error->code == code;
}

void sys_error_setup(void) {
  sys_mutex_init(&g_log_lock);
}

void sys_error_teardown(void) {
  sys_mutex_clear(&g_log_lock);
}
