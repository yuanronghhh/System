#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_MS_DECLARE_STACK(var) \
  SYS_CLEANUP(SysMsMap_cleanup) SysMsMap* var##_PrivateMap; \
  var##_PrivateMap = sys_alloca(sizeof(SysMsMap)); \
  sys_ms_map_construct(var##_PrivateMap, (void **)&var, #var, 0); \
  sys_ms_map_prepend(var##_PrivateMap)

#define SYS_MS_DECLARE_HEAP(var) \
  var##_PrivateMap = sys_ms_map_new_by_addr((void **)&var, #var); \
  sys_ms_map_prepend(var##_PrivateMap) \

#define SYS_MS_PTR(TypeName, var) \
  TypeName* var = SYS_MS_INIT_VALUE; \
  SYS_MS_DECLARE_STACK(var)

#define SYS_MS_STATIC(TypeName, var) \
  static TypeName* var = SYS_MS_INIT_VALUE; \
  static SysMsMap* var##_PrivateMap = NULL

#define SYS_MS_INIT_BEGIN() \
  SYS_CONSTRUCT static void SysMsBlock_statis_init(void) { \

#define SYS_MS_STATIC_INIT(var) \
  var = NULL; \
  SYS_MS_DECLARE_HEAP(var)

#define SYS_MS_INIT_END() }

#define SYS_MS_INIT_WITH_CODE(_CODE_) SYS_MS_INIT_BEGIN() { _CODE_; } SYS_MS_INIT_END()

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
SYS_API void sys_ms_map_prepend(SysMsMap *map);
SYS_API void sys_ms_collect(void);

SYS_END_DECLS

#endif
