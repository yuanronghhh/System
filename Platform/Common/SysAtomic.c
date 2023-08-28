#include <System/Platform/Common/SysAtomic.h>

void sys_ref_count_log(const char *func_name, SysInt i) {
  fprintf(stdout, "[%s] %d\n", func_name, i);
}
