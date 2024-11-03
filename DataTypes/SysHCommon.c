#include <System/DataTypes/SysHCommon.h>
#include <System/Utils/SysError.h>

void sys_hdata_init(SysHData *self) {
  self->check = SYS_HDATA_CHECK_VALUE;
}

SysPointer _sys_hdata_b_cast(SysPointer self, SysInt offsize) {
  if(self == NULL) { return NULL; }

  return (SysChar*)self - offsize;
}

SysPointer _sys_hdata_f_cast(SysPointer self, SysInt offsize) {
  if(self == NULL) { return NULL; }

  return (SysChar*)self + offsize;
}

SysHData* _sys_hdata_cast_check(SysHData *self) {
  sys_return_val_if_fail(SYS_HDATA_CHECK(self), NULL);

  return self;
}
