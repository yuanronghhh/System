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

static void reflect_o1_to_o2(SysObject *o1, SysObject *o2, SysType type1, SysType type2) {
  SysHArray *props1 = sys_object_get_properties(type1);
  SysValue *v1 = NULL;

  for(SysUInt i = 0; i < props1->len; i++) {
    SysParam *p1 = props1->pdata[i];
    const SysChar *name = sys_param_get_field_name(p1);

    if(sys_str_equal(name, "parent")) {
      continue;
    }

    SysParam *p2 = sys_object_get_property(type2, name);
    if (p2 == NULL) {
      continue;
    }

    if(!sys_param_get_value(p1, o1, &v1)) {
      continue;
    }

    if(!sys_param_set_value(p2, o2, v1)) {
    }

    sys_clear_pointer(&v1, sys_value_free);
  }
}

static void test_param_basic(void) {
  SysTestImpl *o1 = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o1);
  o1->width = 1;
  o1->height = 2;

  SysTestImpl *o2 = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o2);
  o2->width = 11;
  o2->height = 12;

  reflect_o1_to_o2(SYS_OBJECT(o1), SYS_OBJECT(o2), SYS_TYPE_TEST_IMPL, SYS_TYPE_TEST_IMPL);
  sys_object_unref(o1);
  sys_object_unref(o2);
}

void test_type_init(int argc, SysChar* argv[]) {
  UNITY_BEGIN();
  {
    // RUN_TEST(test_type_basic);
    RUN_TEST(test_param_basic);

  }
  UNITY_END();
}
