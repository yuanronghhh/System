#include <System/Utils/SysError.h>
#include <System/Utils/SysStr.h>
#include <System/Platform/Common/SysOsPrivate.h>

// static SysInt dev_null = -1;

void sys_real_init_console(void) {
}

SysBool sys_real_console_is_utf8(void) {
  return true;
}

SysBool sys_real_env_set(const SysChar *var, const SysChar *value) {
  SysInt result;

  result = setenv (var, value, true);

  return result == 0;
}

const SysChar* sys_real_env_get(const SysChar *var) {
  sys_return_val_if_fail (var != NULL, NULL);

  return getenv(var);
}

SysUInt64 sys_real_get_monotonic_time(void) {
  struct timespec ts;
  SysInt time;

  time = clock_gettime (CLOCK_MONOTONIC, &ts);

  if (time != 0) {
    sys_abort_N("%s", "clock_gettime error.");
  }

  return (((SysUInt64) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
}

void sys_real_usleep(unsigned long mseconds) {
  struct timespec request, remaining;
  request.tv_sec = mseconds / 1000000;
  request.tv_nsec = 1000 * (mseconds % 1000000);

  while (nanosleep(&request, &remaining) == -1 && errno == EINTR) {
    request = remaining;
  }
}

void* sys_real_dlsymbol(void *handle, const SysChar *symbol) {
  sys_return_val_if_fail(handle != NULL, NULL);
  sys_return_val_if_fail(symbol != NULL, NULL);

  void *p = dlsym (handle, symbol);
  SysChar *msg;

  msg = dlerror();
  if (msg) {
    sys_warning_N("%s", msg);
    return NULL;
  }

  return p;
}

void sys_real_setup(void) {
}

void sys_real_teardown(void) {
}

void* sys_real_dlopen(const SysChar *filename) {
  void *handle = dlopen (filename, RTLD_GLOBAL | RTLD_LAZY);
  if (!handle) {
    sys_warning_N("dlopen failed: %s.", filename);
    return NULL;
  }

  return handle;
}

SysPointer sys_real_dlmodule(const SysChar* name) {
  sys_return_val_if_fail(name != NULL, NULL);

  SysPointer handle = dlopen(name, RTLD_LAZY);
  if (!handle) {
    sys_warning_N("get module failed: %s.", name);
    return NULL;
  }

  return handle;
}

void sys_real_dlclose(void* handle) {
  if(dlclose (handle) != 0) {
    perror("dlclose failed");
  }
}

SysChar **sys_real_backtrace_string(SysInt *size) {
  SysInt stack_size;
  void *stack[SYS_BACKTRACE_SIZE];
  SysChar **s;

  stack_size = backtrace(stack, SYS_BACKTRACE_SIZE);
  s = backtrace_symbols(stack, stack_size);
  if(s == NULL) {
    return NULL;
  }
  *size = stack_size;

  return s;
}

SysInt64 sys_get_real_time (void) {
  struct timeval r;

  /* this is required on alpha, there the timeval structs are ints
   * not longs and a cast only would fail horribly */
  gettimeofday (&r, NULL);

  return (((SysInt64) r.tv_sec) * 1000000) + r.tv_usec;
}

SysChar * sys_os_getlocale (void) {
  const char *locale = setlocale (LC_CTYPE, NULL);

  return sys_strdup(locale);
}

SysInt sys_poll_set_fd (fd_set *fds,
    SysSocket *socket) {
  struct timeval tv;
  fd_set rset, wset, xset;
  fd_set *f;
  int ready;
  int maxfd = 0;

  FD_ZERO (&rset);
  FD_ZERO (&wset);
  FD_ZERO (&xset);

  FD_SET (socket->fd, &rset);
  FD_SET (socket->fd, &wset);
  FD_SET (socket->fd, &xset);

  return ready;
}
