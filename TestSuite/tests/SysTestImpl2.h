#ifndef __SYS_TEST_IMPL2__
#define __SYS_TEST_IMPL2__

#include <System/TestSuite/tests/SysTestImpl.h>

SYS_BEGIN_DECLS

#define SYS_TYPE_TEST_IMPL2 (sys_test_impl2_get_type())
#define SYS_TEST_IMPL2(o) ((SysTestImpl2* )sys_object_cast_check(o, SYS_TYPE_TEST_IMPL2))
#define SYS_TEST_IMPL2_CLASS(o) ((SysTestImpl2Class *)sys_class_cast_check(o, SYS_TYPE_TEST_IMPL2))
#define SYS_TEST_IMPL2_GET_CLASS(o) sys_instance_get_class(o, SysTestImpl2Class)

typedef struct _SysTestImpl2 SysTestImpl2;
typedef struct _SysTestImpl2Class SysTestImpl2Class;

struct _SysTestImpl2 {
  SysTestImpl parent;
  /* < private > */
  SysInt width;
  SysInt height;
};

struct _SysTestImpl2Class {
  SysTestImplClass parent;
};

SysType sys_test_impl2_get_type(void);
SysTestImpl2 *sys_test_impl2_new(void);

SYS_END_DECLS

#endif

