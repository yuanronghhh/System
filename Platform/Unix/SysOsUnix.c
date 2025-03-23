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

SysInt sys_real_poll (SysPollFD *fds,
    SysUInt nfds,
    SysInt timeout)
{
  struct timeval tv;
  fd_set rset, wset, xset;
  int ready;
  int maxfd = 0;
  SysPollFD *f;

  FD_ZERO (&rset);
  FD_ZERO (&wset);
  FD_ZERO (&xset);

  for (f = fds; f < &fds[nfds]; ++f)
    if (f->fd >= 0)
    {
      if (f->events & SYS_POLL_IN)
        FD_SET (f->fd, &rset);
      if (f->events & SYS_POLL_OUT)
        FD_SET (f->fd, &wset);
      if (f->events & SYS_POLL_PRI)
        FD_SET (f->fd, &xset);
      if (f->fd > maxfd && (f->events & (SYS_POLL_IN|SYS_POLL_OUT|SYS_POLL_PRI)))
        maxfd = f->fd;
    }

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  ready = select (maxfd + 1, 
      &rset, 
      &wset, 
      &xset,
      timeout == -1 ? NULL : &tv);

  if (ready > 0)
  {
    for (f = fds; f < &fds[nfds]; ++f)
    {
      f->revents = 0;
      if (f->fd >= 0)
      {
        if (FD_ISSET (f->fd, &rset))
          f->revents |= SYS_POLL_IN;
        if (FD_ISSET (f->fd, &wset))
          f->revents |= SYS_POLL_OUT;
        if (FD_ISSET (f->fd, &xset))
          f->revents |= SYS_POLL_PRI;
      }
    }
  }

  return ready;
}
