#include <System/TestSuite/tests/SysTestImpl.h>
#include <System/TestSuite/tests/SysTestIFace.h>
#include <System/TestSuite/tests/SysTestIFace2.h>

static void sys_test_impl_default(SysTestIFaceInterface* iface);
static void sys_test_impl_default2(SysTestIFace2Interface* iface);

SYS_DEFINE_WITH_CODE(SysTestImpl, sys_test_impl, SYS_TYPE_OBJECT,
  SYS_IMPLEMENT_INTERFACE(SYS_TYPE_TEST_IFACE, sys_test_impl_default)
  SYS_IMPLEMENT_INTERFACE(SYS_TYPE_TEST_IFACE2, sys_test_impl_default2));

static SysInt get_width_i(SysTestIFace *iface) {
  SysTestImpl* self = SYS_TEST_IMPL(iface);

  return self->width;
}

static  SysInt get_height_i(SysTestIFace2 *iface) {
  SysTestImpl* self = SYS_TEST_IMPL(iface);

  return self->height;
}

static void sys_test_impl_default(SysTestIFaceInterface *iface) {
  iface->get_width = get_width_i;
}

static  void sys_test_impl_default2(SysTestIFace2Interface *iface) {
  iface->get_height = get_height_i;
}

SysInt sys_test_impl_get_height(SysTestImpl* self) {
  return self->height;
}

SysInt sys_test_impl_get_width(SysTestImpl* self) {
  return self->width;
}

/* object api */
static void sys_test_impl_dispose(SysObject* o) {

  SYS_OBJECT_CLASS(sys_test_impl_parent_class)->dispose(o);
}

SysTestImpl *sys_test_impl_new(void) {
  return sys_object_new(SYS_TYPE_TEST_IMPL, NULL);
}

static void sys_test_impl_class_init(SysTestImplClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_test_impl_dispose;

  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysObject", SYS_TYPE_OBJECT, parent);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, width);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height1);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height2);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height3);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height4);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height5);
  sys_object_add_property(SYS_TYPE_TEST_IMPL, SysTestImpl, "SysInt", SYS_TYPE_INT, height6);
}

static void sys_test_impl_init(SysTestImpl *self) {
  self->width = 100;
  self->height = 200;
}
