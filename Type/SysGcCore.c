#include <System/Type/SysGcCore.h>
#include <System/Type/SysGcCommonPrivate.h>


void sys_gc_setup(void) {

#if USE_MARKSWEEP
  sys_ms_gc_setup();
#endif
}

void sys_gc_teardown(void) {

#if USE_MARKSWEEP
  sys_ms_gc_teardown();
#endif
}

