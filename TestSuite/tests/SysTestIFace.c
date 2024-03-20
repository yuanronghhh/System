#include <System/TestSuite/tests/SysTestIFace.h>


SYS_DEFINE_INTERFACE(SysTestIFace, sys_test_iface, SYS_TYPE_OBJECT);

void iface_dispose(SysTestIFace* self) {
  // todo
}

void sys_test_iface_dispose(SysTestIFace* self) {
  sys_return_if_fail(self != NULL);

  SysTestIFaceInterface* iface = SYS_TEST_IFACE_GET_IFACE(self);
  TEST_ASSERT_NOT_NULL(iface);
  sys_return_if_fail(iface != NULL);

  iface->dispose(self);
}

SysInt sys_test_iface_get_width(SysTestIFace* self) {
  sys_return_val_if_fail(self != NULL, -1);

  SysTestIFaceInterface* iface = SYS_TEST_IFACE_GET_IFACE(self);
  TEST_ASSERT_NOT_NULL(iface);
  sys_return_val_if_fail(iface != NULL, -1);

  return iface->get_width(self);
}

/* object api */
void sys_test_iface_default_init(SysTestIFaceInterface* iface) {
  iface->dispose = iface_dispose;
}
