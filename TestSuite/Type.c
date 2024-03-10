#include <System/TestSuite/Type.h>
#include <System/TestSuite/tests/SysTestImpl.h>
#include <System/TestSuite/tests/SysTestImpl2.h>
#include <System/TestSuite/tests/SysTestIFace.h>
#include <System/TestSuite/tests/SysTestIFace2.h>


static void test_type_basic(void) {
  SysTestIFace2* iface2;
  SysTestIFace* iface;
  SysInt width, height;

  SysTestImpl *o = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o);

  SysTestImpl2* o2 = sys_test_impl2_new();
  TEST_ASSERT_NOT_NULL(o2);

  iface = SYS_TEST_IFACE(o);
  width = sys_test_iface_get_width(iface);
  TEST_ASSERT_TRUE_MESSAGE(width == 100, "width > 0 must be true");
  sys_debug_N("[impl width] %d", width);
  sys_test_iface_dispose(iface);

  iface2 = SYS_TEST_IFACE2(o);
  height = sys_test_iface2_get_height(iface2);
  TEST_ASSERT_TRUE_MESSAGE(height == 200, "height > 0 must be true");
  sys_debug_N("[impl height] %d", height);

  iface = SYS_TEST_IFACE(o2);
  width = sys_test_iface_get_width(iface);
  TEST_ASSERT_TRUE_MESSAGE(width == 300, "width > 0 must be true");
  sys_debug_N("[impl2 width] %d", width);
  sys_test_iface_dispose(iface);

  iface2 = SYS_TEST_IFACE2(o2);
  height = sys_test_iface2_get_height(iface2);
  TEST_ASSERT_TRUE_MESSAGE(height == 400, "height > 0 must be true");
  sys_debug_N("[impl2 height] %d", height);

  sys_object_unref(o);
  sys_object_unref(o2);
}

static void test_param_basic(void) {
  SysTestImpl *o = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o);

  SysHArray *array = sys_object_get_properties(SYS_TYPE_TEST_IMPL);
  for(SysUInt i = 0; i < array->len; i++) {
    SysParam *param = array->pdata[i];

    sys_debug_N("%s,%d", sys_param_get_name(param), sys_param_get_offset(param));

    SysValue *v = sys_value_new_int(123);

    if(!sys_param_set_value(param, SYS_OBJECT(o), v)) {
    }
  }

  sys_object_unref(o);
}

void test_type_init(int argc, SysChar* argv[]) {
  UNITY_BEGIN();
  {
    // RUN_TEST(test_type_basic);
    RUN_TEST(test_param_basic);

  }
  UNITY_END();
}
