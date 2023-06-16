#ifndef __SYS_MEM_PRIVATE_H__
#define __SYS_MEM_PRIVATE_H__

#include <System/Platform/Common/SysMem.h>

SYS_BEGIN_DECLS

void sys_real_memcpy(
  void*       const dst,
  SysSize         const dst_size,
  void        const* const src,
  SysSize         const src_size);

void sys_real_leaks_init(void);
void sys_real_leaks_report(void);

SYS_END_DECLS

#endif
