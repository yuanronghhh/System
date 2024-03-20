#ifndef __SYS_TEST_IMPL__
#define __SYS_TEST_IMPL__

#include <System/TestSuite/Common.h>

SYS_BEGIN_DECLS

#define SYS_TYPE_TEST_IMPL (sys_test_impl_get_type())
#define SYS_TEST_IMPL(o) ((SysTestImpl* )sys_object_cast_check(o, SYS_TYPE_TEST_IMPL))
#define SYS_TEST_IMPL_CLASS(o) ((SysTestImplClass *)sys_class_cast_check(o, SYS_TYPE_TEST_IMPL))
#define SYS_TEST_IMPL_GET_CLASS(o) sys_instance_get_class(o, SysTestImplClass)

typedef struct _SysTestImpl SysTestImpl;
typedef struct _SysTestImplClass SysTestImplClass;

struct _SysTestImpl {
  SysObject parent;
  /* < private > */
  SysInt width;
  SysInt height;
  SysInt height1;
  SysInt height2;
  SysInt height3;
  SysInt height4;
  SysInt height5;
  SysInt height6;
};

struct _SysTestImplClass {
  SysObjectClass parent;
};

SysType sys_test_impl_get_type(void);
SysTestImpl *sys_test_impl_new(void);
SysInt sys_test_impl_get_height(SysTestImpl* self);
SysInt sys_test_impl_get_width(SysTestImpl *self);

SYS_END_DECLS

#endif

