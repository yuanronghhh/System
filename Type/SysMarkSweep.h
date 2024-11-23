#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysTypeCommon.h>

SYS_BEGIN_DECLS

#define sys_pptr(var) \
  sys_cleanup(_sys_ptr_clean) \
  SysPointer var = sys_alloca(sizeof(SysPointer)); \
  sys_ms_register_adress(&(var))

#define sys_optr(var) \
  sys_cleanup(_sys_object_clean) \
  SysObject* var = sys_alloca(sizeof(SysPointer)); \
  sys_ms_register_adress((SysPointer *)&(var))

#define sys_alloca_s(struct_type) \
  (struct_type)_sys_alloca(sizeof(struct_type))

#define sys_alloca_ptr() \
  sys_alloca(sizeof(SysPointer))

typedef struct _MsBlock MsBlock;
typedef void* MsAddr;

SYS_API void _sys_ptr_clean(SysPointer *o);
SYS_API void _sys_object_clean(SysObject **o);
SYS_API void sys_ms_register_adress(SysPointer *addr);
SYS_API void sys_gc_collect(void);
SYS_API MsBlock *sys_ms_alloc(MsAddr *addr);
SYS_API void sys_ms_setup(void);
SYS_API void sys_ms_teardown(void);

SYS_END_DECLS

#endif
