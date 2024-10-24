#include <System/Utils/SysFile.h>
#include <System/Utils/SysStr.h>
#include <System/DataTypes/SysArray.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysProcessPrivate.h>

#define CloseHandle_N(h) \
do {                                         \
  if (!(CloseHandle(h))) {                   \
    sys_abort_N("%s",  "close handle failed"); \
  }                                          \
} while(0)

typedef struct _SysSubProcessWin32 SysSubProcessWin32;

struct _SysSubProcessWin32 {
  SysSubProcess option;

  STARTUPINFO s_si;
  PROCESS_INFORMATION s_pi;

  HANDLE s_stdin;
  HANDLE s_stdout;
  HANDLE s_stderr;
};

static SysInt devnull = -1;

SysInt sys_real_dev_null(void) {
  SysInt dev_null = sys_open("nul", O_RDWR, -1);
  return dev_null;
}

SysInt sys_real_execvp(const SysChar* path, const SysChar* const args[]) {
  return (SysInt)_execvp(path, args);
}

FILE *sys_real_popen(const SysChar* cmd, const SysChar *mode) {
  FILE *proc = _popen(cmd, mode);
  return proc;
}

static SysBool win32_get_handle(
  PHANDLE rfd0, PHANDLE wfd0,
  PHANDLE rfd1, PHANDLE wfd1,
  PHANDLE rfd2, PHANDLE wfd2,
  SYS_IO_ENUM std0_type, SYS_IO_ENUM std1_type, SYS_IO_ENUM std2_type) {

  HANDLE std0, std1, std2;

  /* stdin */
  if (std0_type == SYS_IO_INVALID) {
    std0 = GetStdHandle(STD_INPUT_HANDLE);

    if (std0 == INVALID_HANDLE_VALUE) {
      if (!CreatePipe(rfd0, wfd0, NULL, 0)) {
        return false;
      }

      CloseHandle_N(*wfd0);
    }

  } else if (std0_type == SYS_IO_PIPE) {
    if (!CreatePipe(rfd0, wfd0, NULL, 0)) {
      return false;
    }

  } else if (std0_type == SYS_IO_DEV_NULL) {
    if (devnull == -1) {
      devnull = sys_real_dev_null();
    }

    *rfd0 = (HANDLE)_get_osfhandle(devnull);
  } else {
    std0 = GetStdHandle(STD_INPUT_HANDLE);
    *rfd0 = std0;
  }

  /* stdout */
  if (std1_type == SYS_IO_INVALID) {
    std1 = GetStdHandle(STD_OUTPUT_HANDLE);
    if (std1 == INVALID_HANDLE_VALUE) {
      if (!CreatePipe(rfd1, wfd1, NULL, 0)) {
        return false;
      }

      CloseHandle_N(*rfd1);
    }

  } else if (std1_type == SYS_IO_PIPE) {
    if (!CreatePipe(rfd1, wfd1, NULL, 0)) {
      return false;
    }

  } else if (std1_type == SYS_IO_DEV_NULL) {
    if (devnull == -1) {
      devnull = sys_real_dev_null();
    }

    *wfd1 = (HANDLE)_get_osfhandle(devnull);
  } else {
    std1 = GetStdHandle(STD_OUTPUT_HANDLE);
    *wfd1 = std1;
  }

  /* stderr */
  if (std2_type == SYS_IO_INVALID) {
    std2 = GetStdHandle(STD_ERROR_HANDLE);
    if (std2 == INVALID_HANDLE_VALUE) {
      if (!CreatePipe(rfd2, wfd2, NULL, 0)) {
        return false;
      }

      CloseHandle_N(*rfd2);
    }

  } else if (std2_type == SYS_IO_PIPE) {
    if (!CreatePipe(rfd2, wfd2, NULL, 0)) {
      return false;
    }
  } else if (std2_type == SYS_IO_STDOUT) {
    *wfd2 = *wfd1;
  } else if (std2_type == SYS_IO_DEV_NULL) {
    if (devnull == -1) {
      devnull = sys_real_dev_null();
    }

    *wfd2 = (HANDLE)_get_osfhandle(devnull);
  } else {
    std2 = GetStdHandle(STD_ERROR_HANDLE);
    *wfd2 = std2;
  }

  return true;
}

static SysBool win32_execute_child(SysSubProcessWin32 *pwin32,
    SysSubProcessOption *option,
    HANDLE rfd0, HANDLE wfd0,
    HANDLE rfd1, HANDLE wfd1,
    HANDLE rfd2, HANDLE wfd2) {
  STARTUPINFO *si = &pwin32->s_si;
  PROCESS_INFORMATION *pi = &pwin32->s_pi;
  SysChar *cmdstr;
  SysSize cmdlen = 0;

  ZeroMemory(si, sizeof(*si));
  si->cb = sizeof(si);
  ZeroMemory(pi, sizeof(*pi));

  si->dwFlags |= STARTF_USESTDHANDLES;
  si->hStdInput = rfd0;
  si->hStdOutput = wfd1;
  si->hStdError = wfd2;

  cmdstr = sys_strjoin_array(" ", option->cmd, &cmdlen);

  if(!CreateProcess(
      NULL,
      cmdstr,                            // command line
      NULL,                              // process security attributes
      NULL,                              // primary thread security attributes
      1,                                 // handles are inherited
      0,                                 // creation flags
      NULL,                              // used environment
      option->cwd,                       // use parent's current directory
      si,                                // STARTUPINFO pointer
      pi)) {
    sys_warning_N("CreateProcess failed (%d). %s \n", GetLastError(), cmdstr);
    return false;
  }
  
  sys_free(cmdstr);
  WaitForSingleObject(pi->hProcess, INFINITE);

  return true;
}

static SysSubProcessWin32 *sys_real_subprocess_internal(SysSubProcessOption* option) {
  sys_return_val_if_fail(option != NULL, NULL);

  SysSubProcessWin32 *pwin32;

  HANDLE std0read = NULL, std0write = NULL;
  HANDLE std1read = NULL, std1write = NULL;
  HANDLE std2read = NULL, std2write = NULL;

  pwin32 = sys_new0(SysSubProcessWin32, 1);

  win32_get_handle(
    &std0read, &std0write,
    &std1read, &std1write,
    &std2read, &std2write,
    option->std0type,
    option->std1type,
    option->std2type);

  win32_execute_child(pwin32,
    option,
    std0read, std0write,
    std1read, std1write,
    std2read, std2write);

  pwin32->s_stdin = std0read;
  pwin32->s_stdout = std1write;
  pwin32->s_stderr = std2write;

  sys_memcpy(&pwin32->option, sizeof(SysSubProcessOption), option, sizeof(SysSubProcessOption));

  return pwin32;
}

SysSubProcess* sys_real_subprocess_new(const SysChar *cmd[]) {
  SysSubProcessOption option = { 0 };

  sys_return_val_if_fail(cmd != NULL, NULL);

  option.cmd = cmd;

 return sys_real_subprocess_new_option(&option);
}

SysSubProcess* sys_real_subprocess_new_option(SysSubProcessOption* option) {
  SysSubProcessWin32 *pwin32;

  sys_return_val_if_fail(option != NULL, NULL);

  pwin32 = sys_real_subprocess_internal(option);
  return (SysSubProcess *)pwin32;
}

void sys_real_subprocess_terminate(SysSubProcess *sub) {
  SysSubProcessWin32 *pwin32;

  sys_return_if_fail(sub != NULL);

  pwin32= (SysSubProcessWin32 *)sub;

  TerminateProcess(pwin32->s_pi.hProcess, 300);
  CloseHandle_N(pwin32->s_pi.hProcess);
  CloseHandle_N(pwin32->s_pi.hThread);

  sys_free(pwin32);
}

SysInt sys_real_pclose(FILE *fp) {
  sys_return_val_if_fail(fp != NULL, -1);

  return _pclose(fp);
}
