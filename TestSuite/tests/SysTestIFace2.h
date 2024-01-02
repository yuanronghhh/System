#ifndef __SYS_TEST_INTERFACE2__
#define __SYS_TEST_INTERFACE2__

#include <System/TestSuite/Common.h>


SYS_BEGIN_DECLS

#define SYS_TYPE_TEST_IFACE2 (sys_test_iface2_get_type())
#define SYS_TEST_IFACE2(o) ((SysTestIFace2* )sys_object_cast_check(o, SYS_TYPE_TEST_IFACE2))
#define SYS_TEST_IFACE2_GET_IFACE(o) (SysTestIFace2Interface *)SYS_TYPE_GET_INTERFACE(o, SYS_TYPE_TEST_IFACE2)

typedef struct _SysTestIFace2 SysTestIFace2;
typedef struct _SysTestIFace2Interface SysTestIFace2Interface;

struct _SysTestIFace2Interface {
  SysTypeInterface parent;

  SysInt (*get_height) (SysTestIFace2 *item);
};

SysType sys_test_iface2_get_type(void);
SysInt sys_test_iface2_get_width(SysTestIFace2* self);

SYS_END_DECLS

#endif

