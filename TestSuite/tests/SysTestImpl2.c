#include <System/TestSuite/tests/SysTestImpl2.h>
#include <System/TestSuite/tests/SysTestIFace.h>
#include <System/TestSuite/tests/SysTestIFace2.h>

static void impl1_default(SysTestIFaceInterface* iface);
static void impl2_default(SysTestIFace2Interface* iface);

SYS_DEFINE_WITH_CODE(SysTestImpl2, sys_test_impl2, SYS_TYPE_TEST_IMPL,
  SYS_IMPLEMENT_INTERFACE(SYS_TYPE_TEST_IFACE, impl1_default)
  SYS_IMPLEMENT_INTERFACE(SYS_TYPE_TEST_IFACE2, impl2_default));

static SysInt get_width_i(SysTestIFace *iface) {
  SysTestImpl2* self = SYS_TEST_IMPL2(iface);

  return self->width;
}

static SysInt get_height_i(SysTestIFace2 *iface) {
  SysTestImpl2* self = SYS_TEST_IMPL2(iface);

  return self->height;
}

static void impl1_default(SysTestIFaceInterface *iface) {
  // iface->get_width = get_width_i;
}

static void impl2_default(SysTestIFace2Interface* iface) {
  iface->get_height = get_height_i;
}

/* object api */
static void sys_test_impl2_dispose(SysObject* o) {

  SYS_OBJECT_CLASS(sys_test_impl2_parent_class)->dispose(o);
}

SysTestImpl2 *sys_test_impl2_new(void) {
  return sys_object_new(SYS_TYPE_TEST_IMPL2, NULL);
}

static void sys_test_impl2_class_init(SysTestImpl2Class* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_test_impl2_dispose;
}

static void sys_test_impl2_init(SysTestImpl2 *self) {
  self->width = 300;
  self->height = 400;
}
