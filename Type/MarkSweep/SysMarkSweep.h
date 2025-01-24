#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_MS_DECLARE_TRACK(var) \
  SYS_VOLATILE void* var##_PrivateMsMap = &(var); \
  UNUSED(var##_PrivateMsMap);

#define SYS_MS_PTR(TypeName, var) \
  TypeName##_SysMsType* var = SYS_MS_INIT_VALUE; \
  SYS_MS_DECLARE_TRACK(var)

#define SYS_MS_STATIC(TypeName, var) \
  static TypeName* var = SYS_MS_INIT_VALUE

#define SYS_MS_CALL(func, ...) \
  func(__VA_ARGS__);

#define SYS_MS_CALL_R(func, ...) ({ \
  SysPointer ret = func(__VA_ARGS__); \
  sys_ms_block_set_type(SYS_MS_BLOCK(ret), SYS_MS_TRACK_AUTO); \
  ret; \
})

#define SYS_MS_DEFINE_TYPE(TypeName) \
  typedef TypeName TypeName##_SysMsType

SYS_MS_DEFINE_TYPE(SysChar);
SYS_MS_DEFINE_TYPE(SysVoid);
SYS_MS_DEFINE_TYPE(SysBool);
SYS_MS_DEFINE_TYPE(SysRef);
SYS_MS_DEFINE_TYPE(SysPointer);
SYS_MS_DEFINE_TYPE(SysInt);
SYS_MS_DEFINE_TYPE(SysShort);
SYS_MS_DEFINE_TYPE(SysUShort);
SYS_MS_DEFINE_TYPE(SysUInt);
SYS_MS_DEFINE_TYPE(SysFloat);
SYS_MS_DEFINE_TYPE(SysVoid);
SYS_MS_DEFINE_TYPE(SysChar);
SYS_MS_DEFINE_TYPE(SysUChar);
SYS_MS_DEFINE_TYPE(SysUInt8);
SYS_MS_DEFINE_TYPE(SysInt8);
SYS_MS_DEFINE_TYPE(SysULong);
SYS_MS_DEFINE_TYPE(SysLong);
SYS_MS_DEFINE_TYPE(SysUIntPtr);
SYS_MS_DEFINE_TYPE(SysDouble);
SYS_MS_DEFINE_TYPE(SysSize);
SYS_MS_DEFINE_TYPE(SysSSize);
SYS_MS_DEFINE_TYPE(SysWChar);

SYS_API SysPointer sys_ms_realloc(SysPointer b, SysSize nsize);
SYS_API SysPointer sys_ms_malloc(SysSize size);
SYS_API void sys_ms_free(void* o);
SYS_API void sys_ms_map_remove(SysMsMap *o);
SYS_API void sys_ms_map_prepend(SysMsMap *map);
#define sys_ms_collect(funcname) \
{ \
  void *caddr = __builtin_frame_address(0); \
  sys_ms_force_collect(funcname, caddr); \
}

SYS_API void sys_ms_force_collect(void *func_addr, void **caddr);

SYS_END_DECLS

#endif
