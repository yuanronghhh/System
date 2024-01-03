#include <System/TestSuite/Type.h>
#include <System/TestSuite/tests/SysTestImpl.h>
#include <System/TestSuite/tests/SysTestIFace.h>
#include <System/TestSuite/tests/SysTestIFace2.h>


static void test_type_basic(void) {
  SysTestImpl *o = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o);

  SysTestIFace* iface = SYS_TEST_IFACE(o);
  SysTestIFace2* iface2 = SYS_TEST_IFACE2(o);

  SysInt width = sys_test_iface_get_width(iface);
  TEST_ASSERT_TRUE_MESSAGE(width > 0, "width > 0 must be true");
  sys_debug_N("[width] %d", width);

  SysInt height = sys_test_iface2_get_height(iface2);
  TEST_ASSERT_TRUE_MESSAGE(height > 0, "width > 0 must be true");
  sys_debug_N("[height] %d", height);

  sys_object_unref(o);
}

void test_type_init(int argc, SysChar* argv[]) {
  UNITY_BEGIN();
  {
    RUN_TEST(test_type_basic);
  }
  UNITY_END();
}
