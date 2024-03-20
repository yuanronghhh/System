#include <System/TestSuite/Main.h>
#include <System/TestSuite/Type.h>
#include <System/TestSuite/Socket.h>


#define CHECK_TYPE SYS_TYPE_OBJECT


void setUp(void) {
}

void tearDown(void) {
}

static void object_new_debug(SysObject *o, const SysChar *name, SysInt ref_count) {
  if (!sys_object_is_a(o, CHECK_TYPE)) {
    return;
  }

  sys_debug_N("%p\t%s\t%d", o, name, ref_count);
  sys_assert(ref_count == 1);
}

static void object_ref_debug(SysObject *o, const SysChar *name, SysInt ref_count) {
  if (!sys_object_is_a(o, CHECK_TYPE)) {
    return;
  }

  sys_debug_N("%p\t%s\t%d", o, name, ref_count);
  sys_assert(ref_count > 0);
}

static void object_unref_debug(SysObject *o, const SysChar *name, SysInt ref_count) {
  if (!sys_object_is_a(o, CHECK_TYPE)) {
    return;
  }

  sys_debug_N("%p\t%s\t%d", o, name, ref_count);
  sys_assert(ref_count > 0);
}

int main(int argc, SysChar* argv[]) {
  sys_setup();

#if 0
  sys_object_set_ref_hook(object_ref_debug);
  sys_object_set_unref_hook(object_unref_debug);
  sys_object_set_new_hook(object_new_debug);
#endif

  test_type_init(argc, argv);

  sys_teardown();
  return 0;
}
