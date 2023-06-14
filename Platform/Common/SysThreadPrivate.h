#ifndef __SYS_THREAD_PRIVATE_H__
#define __SYS_THREAD_PRIVATE_H__

#include <System/Platform/Common/SysThread.h>

SYS_BEGIN_DECLS

typedef struct _SysRealThread SysRealThread;

struct _SysRealThread {
  SysThread thread;
  SysPointer retval;
  SysRef ref_count;
};

SYS_END_DECLS

#endif
