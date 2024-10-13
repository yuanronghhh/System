#include <System/Utils/SysStr.h>
#include <System/DataTypes/SysQuark.h>
#include <System/Utils/SysErrorPrivate.h>

/**
 * sys_real_strerr:
 * @errnum: errno
 *
 * Returns: const string.
 */
const SysChar *sys_real_strerr(SysInt errnum) {
  SysChar msg[1024] = {0};
  const SysChar *qmsg;

  qmsg = sys_quark_string(&msg[0]);
  errno_t eno = strerror_s(&msg[0], sizeof(msg), errnum);

  if (eno != 0) {
    sys_abort_N("%s", "strerror_s failed");
  }

  return qmsg;
}
