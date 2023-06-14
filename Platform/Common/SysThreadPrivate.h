#ifndef __SYS_THREAD_PRIVATE_H__
#define __SYS_THREAD_PRIVATE_H__

#include <System/Platform/Common/SysThread.h>

SYS_BEGIN_DECLS

typedef struct _SysRealThread SysRealThread;

struct _SysRealThread {
  SysThread thread;

  SysInt ref_count;
  SysBool ours;
  SysChar *name;
  SysPointer retval;
};

SYS_END_DECLS

#endif
