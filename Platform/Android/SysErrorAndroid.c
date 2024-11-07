#include <System/Utils/SysStr.h>
#include <System/Utils/SysErrorPrivate.h>

static SysChar buff[1024];
/**
 * sys_real_strerr:
 * @errnum: errno
 *
 * Returns: Void
 */
const SysChar* sys_real_strerr(SysInt errnum) {
  SysChar* err = strerror_r(errnum, buff, sizeof(buff));

  return err;
}
