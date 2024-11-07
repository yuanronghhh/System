#include <System/Utils/SysErrorPrivate.h>
#include <System/Utils/SysStr.h>

static SysChar buff[1024];
/**
 * sys_real_strerr:
 * @errnum: errno
 *
 * Returns: error string
 */
const SysChar* sys_real_strerr(SysInt errnum) {
  SysChar *err = strerror_r(errnum, buff, sizeof(buff));

  return err;
}

SysChar * sys_error_real_message (SysInt error) {

  return sys_strdup(sys_real_strerr(error));
}
