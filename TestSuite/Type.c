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

typedef struct _SysPMap SysPMap;
struct _SysPMap {
  SysInt p1;
  SysInt p2;
};

static SysHArray *reflect_create_pmap(SysType type1, SysType type2) {
  SysHArray *pmaps = sys_harray_new_with_free_func(sys_free);
  SysHArray *props1 = sys_object_get_properties(type1);
  SysParam *p1;
  SysPMap *pmap;

  for(SysUInt i = 0; i < props1->len; i++) {
    p1 = props1->pdata[i];

    if (sys_str_equal(p1->field_name, "parent")) {
      continue;
    }

    pmap = sys_new0_N(SysPMap, 1);
    pmap->p1 = p1->offset;
    pmap->p2 = p1->offset;

    sys_harray_add(pmaps, pmap);
  }

  return pmaps;
}

static inline void reflect_o1_to_o2(SysObject *o1, SysObject *o2, SysHArray *pmaps) {
  SysPMap *map;

  for(SysUInt i = 0; i < pmaps->len; i++) {
    map = pmaps->pdata[i];

    *((SysUInt8 *)o2 + map->p1) = *((SysUInt8 *)o1 + map->p1);
  }
}

static void test_param_basic(void) {
  SysUInt64 start, end, reflect_tm, direct_tm;
  SysHArray *pmaps;

  SysTestImpl *o1 = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o1);
  o1->width = 1;
  o1->height = 2;
  o1->height2 = 2;
  o1->height3 = 2;
  o1->height4 = 2;
  o1->height5 = 2;
  o1->height6 = 2;

  SysTestImpl *o2 = sys_test_impl_new();
  TEST_ASSERT_NOT_NULL(o2);
  o2->width = 11;
  o2->height = 12;

  SysPMap *map;

  pmaps = reflect_create_pmap(SYS_TYPE_TEST_IMPL, SYS_TYPE_TEST_IMPL);
  start = sys_get_monotonic_time();
  for(SysUInt j = 0; j < pmaps->len; j++) {
    map = pmaps->pdata[j];

    for(SysInt i = 0; i < 100000000; i++) {
      *((SysUInt8 *)o2 + map->p1) = *((SysUInt8 *)o1 + map->p1);
    }
  }
  end = sys_get_monotonic_time();
  reflect_tm = end - start;

  start = sys_get_monotonic_time();
  for(SysInt i = 0; i < 100000000; i++) {
    o2->width = o1->width;
    o2->height = o1->height;
    o2->height1 = o1->height1;
    o2->height2 = o1->height2;
    o2->height3 = o1->height3;
    o2->height4 = o1->height4;
    o2->height5 = o1->height5;
    o2->height6 = o1->height6;
  }
  end = sys_get_monotonic_time();
  direct_tm = end - start;

  sys_debug_N("%d,%d", reflect_tm, direct_tm);
  sys_debug_N("%d", o2->height6);

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
