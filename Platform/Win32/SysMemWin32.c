#include <System/Utils/SysFile.h>
#include <System/Utils/SysString.h>
#include <System/DataTypes/SysArray.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysOs.h>
#include <System/Platform/Common/SysMemPrivate.h>

#define MAXMODULELISTLENGTH 512

void sys_real_memcpy(
  void*       const dst,
  SysSize     const dst_size,
  void const* const src,
  SysSize     const src_size) {
  errno_t eno = memcpy_s(dst, dst_size, src, src_size);

  if (eno) {
    sys_abort_N("%s", "memcpy_s failed");
  }
}

void* sys_real_aligned_malloc(SysSize align, SysSize size) {

  return _aligned_malloc(size, align);
}

void sys_real_aligned_free(void* ptr) {

  _aligned_free(ptr);
}

SysSize sys_real_get_msize(void* block) {

  return _msize(block);
}

void sys_real_leaks_init(void) {
#if USE_DEBUGGER
  VLDSetOptions(VLD_OPT_SKIP_CRTSTARTUP_LEAKS
      | VLD_OPT_AGGREGATE_DUPLICATES
      | VLD_OPT_MODULE_LIST_INCLUDE
      | VLD_OPT_VALIDATE_HEAPFREE
      , 0, 0);
#endif
}

void sys_real_leaks_report(void) {
#if USE_DEBUGGER
  wchar_t *wname;
  SysInt size;
  const SysChar *leakfile;

  sys_print("Closing file handle for leak report.\n");
  sys_fcloseall();

  leakfile = sys_leaks_get_file();
  if(leakfile) {

    SYS_LEAK_IGNORE_BEGIN;
    wname = sys_mbyte_to_wchar(leakfile, &size);
    SYS_LEAK_IGNORE_END;

    VLDSetReportOptions(VLD_OPT_REPORT_TO_FILE, wname);
    VLDReportLeaks();
    sys_free(wname);
  } else {

    VLDReportLeaks();
    getchar();
  }
#endif
}

