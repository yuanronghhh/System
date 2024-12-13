#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_MS_DECLARE_STACK(var) \
  sys_cleanup(SysMsMap_cleanup) SysMsMap* var##_PrivateMap; \
  var##_PrivateMap = sys_alloca(sizeof(SysMsMap)); \
  sys_ms_map_construct(var##_PrivateMap, (void **)&var, #var, NULL); \
  sys_ms_register_map(var##_PrivateMap) \

#define SYS_MS_DECLARE_HEAP(var) \
  var##_PrivateMap = sys_ms_map_new_by_addr((void **)&var, #var); \
  sys_ms_register_map(var##_PrivateMap) \

#define SYS_MS_PTR(TypeName, var) \
  TypeName* var = SYS_MS_INIT_VALUE; \
  SYS_MS_DECLARE_STACK(var)

#define SYS_MS_STATIC(TypeName, var) \
  static TypeName* var = SYS_MS_INIT_VALUE; \
  static SysMsMap* var##_PrivateMap = NULL

#define SYS_MS_STATIC_INIT(var) \
  SYS_MS_DECLARE_HEAP(var)

#define SYS_MS_STATIC_CLEAR(var) \
  SysMsMap_cleanup(&(var##_PrivateMap))

#define SYS_MS_CALL(func, ...) ({ \
  SysPointer ret = func(__VA_ARGS__); \
  sys_ms_block_set_type(SYS_MS_BLOCK(ret), SYS_MS_TRACK_AUTO); \
  ret; \
})

SYS_API SysPointer sys_ms_realloc(SysPointer b, SysSize nsize);
SYS_API SysPointer sys_ms_malloc(SysSize size);
SYS_API void sys_ms_free(void* o);
SYS_API void sys_ms_map_remove(SysMsMap *o);
SYS_API void sys_ms_unregister_var(SysMsMap **addr);
SYS_API void sys_ms_register_map(SysMsMap *map);
SYS_API void sys_ms_unregister_map(SysMsMap *map);
SYS_API void sys_ms_collect(void);

SYS_END_DECLS

#endif
