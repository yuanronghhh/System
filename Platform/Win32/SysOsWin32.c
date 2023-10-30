#include <System/Platform/Common/SysOsPrivate.h>
#include <System/Utils/SysString.h>
#include <System/DataTypes/SysArray.h>
#include <System/Platform/Common/SysThread.h>


BOOL WINAPI DllMain(HINSTANCE hinstDLL,
  DWORD     fdwReason,
  LPVOID    lpvReserved);

HMODULE win32dll;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason, LPVOID lpvReserved) {
  UNUSED(lpvReserved);

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      win32dll = hinstDLL;
      sys_thread_init();
      break;

    case DLL_THREAD_DETACH:
      sys_thread_detach ();
      break;

    case DLL_PROCESS_DETACH:
      // sys_process_detach();
      break;
    default:
      break;
  }

  return true;
}

void sys_real_init_console(void) {
  SetConsoleOutputCP(65001);

  CONSOLE_FONT_INFOEX cfi;
  cfi.cbSize = sizeof(cfi);
  cfi.nFont = 0;
  cfi.dwFontSize.X = 0;
  cfi.dwFontSize.Y = 16;
  cfi.FontFamily = FF_DONTCARE;
  cfi.FontWeight = FW_NORMAL;
  wcscpy_s(cfi.FaceName, 9, L"Consolas");
  SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), false, &cfi);

  /* enable ansci color */
  SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), 7);
  SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), 7);

#if 0
  /* resize console */
  SMALL_RECT rc = { 0, 0, 120, 30 };
  SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &rc);
#endif
}

bool sys_real_console_is_utf8(void) {
  SysUInt cp = GetConsoleOutputCP();

  if (cp == 65001) {
    return true;
  }

  return false;
}

/**
 * sys_real_get_env: get system enviroment
 * @var: name
 * need call sys_free_N after used
 *
 * Returns: new allocted string.
 */
const SysChar *sys_real_get_env(const SysChar *var) {
  wchar_t *wvalue;
  wchar_t tmp[2];
  wchar_t *wname;
  const SysChar *value;
  int len;

  wname = sys_mbyte_to_wchar(var, NULL);
  len = GetEnvironmentVariableW(wname, tmp, 2);
  if (len == 0) {
    sys_free_N(wname);
    return NULL;
  }

  if (len == 1) {
    len = 2;
  }

  wvalue = sys_new_N(wchar_t, len);

  if ((int)GetEnvironmentVariableW(wname, wvalue, len) != (len - 1)) {
    sys_free_N(wname);
    sys_free_N(wvalue);
    return NULL;
  }

  value = sys_wchar_to_mbyte(wvalue, NULL);

  sys_free_N(wname);
  sys_free_N(wvalue);

  return value;
}

bool sys_real_set_env(const SysChar *var, const SysChar *value) {
  wchar_t *wvar, *wvalue;
  const wchar_t *wname;
  SysChar *nvar;
  int ret;
  SysSize vlen;

  vlen = strlen(var);

  wname = sys_mbyte_to_wchar(var, NULL);
  wvalue = sys_mbyte_to_wchar(value, NULL);
  nvar = sys_new0_N(SysChar, vlen + 2);
  sys_memcpy(nvar, vlen + 2, var, vlen);
  sys_memcpy(nvar + vlen, vlen + 2, "=", 1);

  wvar = sys_mbyte_to_wchar(nvar, NULL);
  _wputenv(wvar);

  ret = (SetEnvironmentVariableW(wname, wvalue) != 0);

  sys_free_N(nvar);
  sys_free_N(wvar);

  sys_free_N((void *)wname);
  sys_free_N(wvalue);

  return ret;
}

SysUInt64 sys_real_get_monotonic_time(void) {
  SysUInt64 ticks;

  ticks = GetTickCount64();

  return ticks;
}

void sys_real_usleep(unsigned long mseconds) {
  /* Round up to the next millisecond */
  Sleep(mseconds ? (1 + (mseconds - 1) / 1000) : 0);
}

SysPointer sys_real_dlopen(const SysChar *filename) {
  sys_return_val_if_fail(filename != NULL, NULL);

  HINSTANCE handle;
  wchar_t *wname;

  wname = sys_mbyte_to_wchar(filename, NULL);
  handle = LoadLibraryW(wname);
  sys_free_N(wname);

  if (!handle) {
    sys_warning_N("dlopen failed: %s.", filename);
    return NULL;
  }

  return handle;
}

SysPointer sys_real_dlsymbol(void *handle, const SysChar *symbol) {
  sys_return_val_if_fail(handle != NULL, NULL);
  sys_return_val_if_fail(symbol != NULL, NULL);

  SysPointer p = (SysPointer)GetProcAddress(handle, symbol);
  if (!p) {
    sys_warning_N("dlsymbol failed: %s.", symbol);
    return NULL;
  }

  return p;
}

void sys_real_dlclose(void* handle) {
  sys_return_if_fail(handle != NULL);

  FreeLibrary(handle);
}

SysChar **sys_real_backtrace_string(SysInt *size) {
  SysInt i;
  SysInt frame_size, rsize;
  SysChar **s, **p;
  IMAGEHLP_SYMBOL64 *symbol;
  void *stack[SYS_BACKTRACE_SIZE];

  HANDLE         process;
  int dp = 0;

  IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
  process = GetCurrentProcess();

  SymInitialize(process, NULL, true);

  frame_size = CaptureStackBackTrace(2, SYS_BACKTRACE_SIZE, stack, NULL);
  if (frame_size < 4) {
    return NULL;
  }
  rsize = frame_size - 3;

  s = sys_new_N(SysChar *, rsize);
  p = s;

  symbol = (IMAGEHLP_SYMBOL64 *)sys_malloc_N(sizeof(IMAGEHLP_SYMBOL64) + 256 * sizeof(char));
  symbol->MaxNameLength = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  for (i = 0; i < rsize; i++) {
    DWORD_PTR addr = (DWORD_PTR)stack[i];

    SymGetSymFromAddr64(process, addr, 0, symbol);
    SymGetLineFromAddr(process, addr, (DWORD *)&dp, &lineInfo);

    *p++ = sys_strdup_printf("%s:%d:%s - %p\n",  lineInfo.FileName, lineInfo.LineNumber, symbol->Name, symbol->Address);
  }
  sys_free_N(symbol);

  *size = rsize;

  return s;
}

void sys_real_setup(void) {
  WSADATA info;
  if (WSAStartup(MAKEWORD(1, 1), &info) != 0) {
    sys_abort_N("%s", "WSAStartup() init for sockect failed");
  }
}

void sys_real_teardown(void) {
}

