#include <System/SysCore.h>
#include <System/Platform/Common/SysOsPrivate.h>
#include <System/DataTypes/SysQuark.h>

static SysBool inited = false;

void sys_setup(void) {
  if(inited) {return;}

  sys_console_setup();
  sys_error_setup();
  sys_thread_init();
  sys_real_setup();
  sys_ssl_setup();
  sys_quark_setup();
  sys_leaks_setup();
  sys_type_setup();

  inited = true;
}

void sys_teardown(void) {
  if(!inited) {return;}

  sys_type_teardown();
  sys_real_teardown();
  sys_quark_teardown();
  sys_ssl_teardown();
  sys_thread_detach();
  sys_leaks_report();
  sys_error_teardown();

  inited = false;
}
