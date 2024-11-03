#ifndef __SYS_HCOMMON_H__
#define __SYS_HCOMMON_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#define SYS_HDATA_CHECK_VALUE 0xdeadbeef
#define SYS_HDATA_CAST_TO(o, TypeName, member) (TypeName *)_sys_hdata_cast_to(o, offsetof(TypeName, member))
#define SYS_HDATA_CHECK(o) (o != NULL && (o)->check == SYS_HDATA_CHECK_VALUE)
#define SYS_HDATA(o, TypeName) ((TypeName *)o)
#define SYS_DATA_TO_HDATA(o, TypeName, member) ((TypeName *)&(o->member))

struct _SysHData {
  SysUInt check;
};

SysPointer _sys_hdata_cast_to(SysHData *self, SysInt offsize);
void sys_hdata_init(SysHData *self);

SYS_END_DECLS

#endif
