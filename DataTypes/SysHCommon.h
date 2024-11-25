#ifndef __SYS_HCOMMON_H__
#define __SYS_HCOMMON_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#define SYS_HDATA_CHECK_VALUE 0xCCCCCCCC
#define SYS_HDATA_CHECK(o) (o != NULL && (o)->check == SYS_HDATA_CHECK_VALUE)
#define SYS_HDATA_F_CAST(o, TypeName, member) ((TypeName *)_sys_hdata_f_cast(o, offsetof(TypeName, member)))
#define SYS_HDATA_B_CAST(o, TypeName, member) ((TypeName *)_sys_hdata_b_cast(o, offsetof(TypeName, member)))
#define SYS_HDATA(o) _sys_hdata_cast_check((SysHData *)o)
#define SYS_IS_HDATA(o) SYS_HDATA_CHECK((SysHData *)o)

struct _SysHData {
  SysSize check;
};

SysPointer _sys_hdata_b_cast(SysPointer self, SysInt offsize);
SysPointer _sys_hdata_f_cast(SysPointer self, SysInt offsize);
SysHData* _sys_hdata_cast_check(SysHData *self);
void sys_hdata_init(SysHData *self);

SYS_END_DECLS

#endif
