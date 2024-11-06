#include <System/Utils/SysStr.h>
#include <System/DataTypes/SysQuark.h>
#include <System/Utils/SysUtf8.h>
#include <System/Utils/SysErrorPrivate.h>

/**
 * sys_real_strerr:
 * @errnum: errno
 *
 * Returns: const string.
 */
const SysChar *sys_real_strerr(SysInt errnum) {
  const SysChar *qmsg;
  errno_t eno;
  SysChar msg[1024] = {0};

  qmsg = sys_quark_string(&msg[0]);
  eno = strerror_s(&msg[0], sizeof(msg), errnum);

  if (eno != 0) {
    sys_abort_N("%s", "strerror_s failed");
  }

  return qmsg;
}

SysChar * sys_error_real_message (SysInt error) {
  SysChar *retval;
  wchar_t *msg = NULL;
  size_t nchars;

  FormatMessageW (FORMAT_MESSAGE_ALLOCATE_BUFFER
                  |FORMAT_MESSAGE_IGNORE_INSERTS
                  |FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, error, 0,
                  (LPWSTR) &msg, 0, NULL);
  if (msg != NULL)
    {
      nchars = wcslen (msg);

      if (nchars >= 2 && msg[nchars-1] == L'\n' && msg[nchars-2] == L'\r')
        msg[nchars-2] = L'\0';

      retval = sys_utf16_to_utf8 (msg, -1, NULL, NULL, NULL);

      LocalFree (msg);
    }
  else
    retval = sys_strdup ("");

  return retval;
}
