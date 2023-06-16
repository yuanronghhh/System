#include <System/Utils/SysString.h>
#include <System/Utils/SysErrorPrivate.h>

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

  if (eno != 0) {
    sys_abort_N("%s", "strerror_s failed");
  }

  return msg;
}
