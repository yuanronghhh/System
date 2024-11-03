#include <System/DataTypes/SysHCommon.h>
#include <System/Utils/SysError.h>

void sys_hdata_init(SysHData *self) {
  self->check = SYS_HDATA_CHECK_VALUE;
}

SysPointer _sys_hdata_cast_to(SysHData *self, SysInt offsize) {
  sys_return_val_if_fail(SYS_HDATA_CHECK(self), NULL);

  return ((SysChar*)self - offsize);
}
