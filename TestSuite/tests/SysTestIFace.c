#include <System/TestSuite/tests/SysTestIFace.h>

void sys_test_iface_default_init(SysTestIFaceInterface* iface);

SYS_DEFINE_INTERFACE(SysTestIFace, sys_test_iface, SYS_TYPE_OBJECT);

void sys_test_iface_default_init(SysTestIFaceInterface* iface) {
}

SysInt sys_test_iface_get_width(SysTestIFace* self) {
  sys_return_val_if_fail(self != NULL, -1);

  SysTestIFaceInterface* iface = SYS_TEST_IFACE_GET_IFACE(self);
  TEST_ASSERT_NOT_NULL(iface);

  sys_return_val_if_fail(iface != NULL, -1);

  return iface->get_width(self);
}
