#include <Utils/SysString.h>
#include <Utils/SysErrorPrivate.h>

static SysChar buff[1024];

/**
 * sys_real_strerr:
 * @errnum: errno
 *
 * Returns: const string.
 */
const SysChar *sys_real_strerr(SysInt errnum) {
  SysChar *msg = &buff[0];

  errno_t eno = strerror_s(msg, sizeof(buff), errnum);
  sys_abort_E(eno == 0, "strerror_s failed");

  return msg;
}
