#include <System/SysCore.h>
#include <System/Platform/Common/SysOsPrivate.h>
#include <System/DataTypes/SysQuark.h>

static SysBool inited = false;

void sys_setup(void) {
  if(inited) {return;}

  sys_console_setup();
  sys_thread_init();
  sys_error_setup();
  sys_real_setup();
#if USE_OPENSSL
  sys_ssl_setup();
#endif
  sys_mem_setup();
  sys_gc_setup();
  sys_quark_setup();
  sys_type_setup();

  inited = true;
}

void sys_teardown(void) {
  if(!inited) {return;}

  sys_type_teardown();
  sys_quark_teardown();
  sys_real_teardown();
  sys_gc_teardown();
#if USE_OPENSSL
  sys_ssl_teardown();
#endif
  sys_error_teardown();
  sys_thread_detach();
  sys_mem_teardown();

  inited = false;
}
