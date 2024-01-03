#include <System/TestSuite/tests/SysTestIFace2.h>


SYS_DEFINE_INTERFACE(SysTestIFace2, sys_test_iface2, SYS_TYPE_OBJECT);

/* object api */
void sys_test_iface2_default_init(SysTestIFace2Interface* iface2) {
}

SysInt sys_test_iface2_get_height(SysTestIFace2 *self) {
  sys_return_val_if_fail(self != NULL, -1);

  SysTestIFace2Interface* iface2 = SYS_TEST_IFACE2_GET_IFACE(self);
  TEST_ASSERT_NOT_NULL(iface2);
  sys_return_val_if_fail(iface2 != NULL, -1);

  return iface2->get_height(self);
}
