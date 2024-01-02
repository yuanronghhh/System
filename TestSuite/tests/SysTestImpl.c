#include <System/TestSuite/tests/SysTestImpl.h>
#include <System/TestSuite/tests/SysTestIFace.h>
#include <System/TestSuite/tests/SysTestIFace2.h>

void sys_test_impl_default(SysTestIFaceInterface* iface);
void sys_test_impl_default2(SysTestIFace2Interface* iface);

SYS_DEFINE_WITH_CODE(SysTestImpl, sys_test_impl, SYS_TYPE_OBJECT,
  SYS_IMPLEMENT_INTERFACE(SYS_TYPE_TEST_IFACE, sys_test_impl_default)
  SYS_IMPLEMENT_INTERFACE(SYS_TYPE_TEST_IFACE2, sys_test_impl_default2));

SysInt get_width_i(SysTestIFace *iface) {
  SysTestImpl* self = SYS_TEST_IMPL(iface);

  return 2;
}

SysInt get_height_i(SysTestIFace2 *iface) {
  SysTestImpl* self = SYS_TEST_IMPL(iface);

  return 4;
}

void sys_test_impl_default(SysTestIFaceInterface *iface) {
  iface->get_width = get_width_i;
}

void sys_test_impl_default2(SysTestIFace2Interface *iface) {
  iface->get_height = get_height_i;
}


/* object api */
static void sys_test_impl_dispose(SysObject* o) {
  SysTestImpl* self = SYS_TEST_IMPL(o);

  SYS_OBJECT_CLASS(sys_test_impl_parent_class)->dispose(o);
}

SysTestImpl *sys_test_impl_new(void) {
  return sys_object_new(SYS_TYPE_TEST_IMPL, NULL);
}

static void sys_test_impl_class_init(SysTestImplClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_test_impl_dispose;
}

static void sys_test_impl_init(SysTestImpl *self) {
}
