#include <System/Platform/Common/SysFile.h>
#include <System/Platform/Common/SysProcessPrivate.h>

typedef struct _SysSubProcessPosix SysSubProcessPosix;

struct _SysSubProcessPosix {
  SysSubProcess parent;

  FILE* s_stdin;
  FILE* s_stdout;
  FILE* s_stderr;
};

// static SysInt devnull = -1;

SysInt sys_real_dev_null(void) {
  SysInt dev_null = sys_open("/dev/null", O_RDWR, -1);
  return dev_null;
}

SysInt sys_real_execvp(const SysChar* path, const SysChar* const args[]) {
  return (SysInt)execvp(path, (SysChar *const *)args);
}

FILE *sys_real_popen(const SysChar* cmd, const SysChar *mode) {
  FILE *proc = popen(cmd, mode);
  return proc;
}

static SysBool posix_get_handle(
  FILE* rfd0, FILE* wfd0,
  FILE* rfd1, FILE* wfd1,
  FILE* rfd2, FILE* wfd2,
  SYS_IO_ENUM std0_type, SYS_IO_ENUM std1_type, SYS_IO_ENUM std2_type) {

#if 0
  FILE* std0, *std1, *std2;

  /* stdin */
  if (std0_type == SYS_IO_INVALID) {
    std0 = sys_fopen(stdin);

    if (std0 == INVALID_HANDLE_VALUE) {
      if (!pipe(rfd0, NULL, 0)) {
        return false;
      }

      fclose(*wfd0);
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

  } else if (std2_type == SYS_IO_DEV_NULL) {
    if (devnull == -1) {
      devnull = sys_real_dev_null();
    }

    *wfd2 = (HANDLE)_get_osfhandle(devnull);
  }
#endif

  return true;
}

static SysBool posix_execute_child(SysSubProcessPosix *pposix,
    SysSubProcessOption *option,
    FILE * rfd0, FILE * wfd0,
    FILE * rfd1, FILE * wfd1,
    FILE * rfd2, FILE * wfd2) {
#if 0
  STARTUPINFO *si = &pwin32->s_si;
  PROCESS_INFORMATION *pi = &pwin32->s_pi;
  SysChar *cmdstr;

  ZeroMemory(si, sizeof(si));
  si->cb = sizeof(si);
  ZeroMemory(pi, sizeof(pi));

  si->dwFlags |= STARTF_USESTDHANDLES;
  si->hStdInput = rfd0;
  si->hStdOutput = wfd1;
  si->hStdError = wfd2;

  cmdstr = sys_strjoin(",", option->cmd, option->cmdlen);

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
    sys_warning_N("CreateProcess failed (%d).\n", GetLastError());
    return false;
  }

  WaitForSingleObject(pi->hProcess, INFINITE);
#endif

  return true;
}

static SysSubProcessPosix *sys_real_subprocess_internal(SysSubProcessOption* option) {
  SysSubProcessPosix *pposix = sys_new0(SysSubProcessPosix, 1);

  FILE* std0read = NULL;
  FILE* std0write = NULL;
  FILE* std1read = NULL;
  FILE* std1write = NULL;
  FILE* std2read = NULL;
  FILE* std2write = NULL;

  posix_get_handle(
    std0read, std0write,
    std1read, std1write,
    std2read, std2write,
    option->std0type,
    option->std1type,
    option->std2type);

  posix_execute_child(pposix,
    option,
    std0read, std0write,
    std1read, std1write,
    std2read, std2write);

  pposix->s_stdin = std0read;
  pposix->s_stdout = std1write;
  pposix->s_stderr = std2write;

  return pposix;
}

SysSubProcess* sys_real_subprocess_new(const SysChar *cmd[]) {
  SysSubProcessOption option = { 0 };

  option.cmd = cmd;

 return sys_real_subprocess_new_option(&option);
}

SysSubProcess* sys_real_subprocess_new_option(SysSubProcessOption* option) {
  SysSubProcessPosix *pposix = sys_real_subprocess_internal(option);

  return (SysSubProcess *)pposix;
}

void sys_real_subprocess_terminate(SysSubProcess *sub) {
  SysSubProcessPosix *pposix = (SysSubProcessPosix *)sub;

  sys_fclose(pposix->s_stdin);
  sys_fclose(pposix->s_stdout);
  sys_fclose(pposix->s_stderr);
}

SysInt sys_real_pclose(FILE *fp) {
  return pclose(fp);
}
