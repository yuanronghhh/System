#ifndef __SYS_TEST_INTERFACE__
#define __SYS_TEST_INTERFACE__

#include <System/TestSuite/Common.h>


SYS_BEGIN_DECLS

#define SYS_TYPE_TEST_IFACE (sys_test_iface_get_type())
#define SYS_TEST_IFACE(o) ((SysTestIFace* )sys_object_cast_check(o, SYS_TYPE_TEST_IFACE))
#define SYS_TEST_IFACE_GET_IFACE(o) (SysTestIFaceInterface *)SYS_TYPE_GET_INTERFACE(o, SYS_TYPE_TEST_IFACE)

typedef struct _SysTestIFace SysTestIFace;
typedef struct _SysTestIFaceInterface SysTestIFaceInterface;

struct _SysTestIFaceInterface {
  SysTypeInterface parent;

  void (*dispose) (SysTestIFace* item);
  SysInt (*get_width) (SysTestIFace *item);
};

SysType sys_test_iface_get_type(void);
SysInt sys_test_iface_get_width(SysTestIFace* self);
void sys_test_iface_dispose(SysTestIFace* self);

SYS_END_DECLS

#endif

