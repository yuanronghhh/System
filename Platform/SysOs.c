#include <Utils/SysString.h>
#include <Utils/SysError.h>
#include <Platform/SysOsPrivate.h>

void sys_init_console(void) {
  sys_real_init_console();
}

SysUInt sys_nearest_pow(SysUInt num) {
  SysUInt n = 1;

  while (n < num && n > 0) {
    n <<= 1;
  }

  return n;
}

bool sys_console_is_utf8(void) {
  return sys_real_console_is_utf8();
}

bool sys_set_env(const char *var, const char *value) {
  return sys_real_set_env(var, value);
}

const char* sys_get_env(const char *var) {
  return sys_real_get_env(var);
}

SysUInt64 sys_get_monoic_time(void) {
  return sys_real_get_monoic_time();
}

void sys_usleep(unsigned long mseconds) {
  sys_real_usleep(mseconds);
}

void* sys_dlopen(const char *filename) {
  return sys_real_dlopen(filename);
}

void* sys_dlsymbol(void *handle, const char *symbol) {
  return sys_real_dlsymbol(handle, symbol);
}

void sys_dlclose(void* handle) {
  sys_real_dlclose(handle);
}

void sys_init(void) {
	sys_real_init();
}
