#include <Utils/SysString.h>
#include <Utils/SysErrorPrivate.h>

static SysChar buff[1024];
/**
 * sys_real_strerr:
 * @errnum: errno
 *
 * Returns: Void
 */
SysChar* sys_real_strerr(SysInt errnum) {
  SysChar *msg = strerror_r(errnum, buff, sizeof(buff));

  return msg;
}
