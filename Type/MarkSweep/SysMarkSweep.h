#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_MS_DECLARE_STACK(var) \
  sys_cleanup(SysMsMap_cleanup) SysMsMap* var##_PrivateMap; \
  var##_PrivateMap = sys_alloca(sizeof(SysMsMap)); \
  sys_ms_map_construct(var##_PrivateMap, (void **)&var, NULL); \
  sys_ms_register_map(var##_PrivateMap) \

#define SYS_MS_DECLARE_HEAP(var) \
  sys_cleanup(SysMsMap_cleanup) SysMsMap* var##_PrivateMap; \
  var##_PrivateMap = sys_ms_map_new_by_addr((void **)&var); \
  sys_ms_register_map(var##_PrivateMap) \

#define SYS_DECLARE_PTR(TypeName, var) \
  TypeName* var = SYS_MS_INIT_VALUE; \
  SYS_MS_DECLARE_STACK(var)

SYS_API void sys_ms_map_remove(SysMsMap *o);
SYS_API void sys_ms_unregister_var(SysMsMap **addr);
SYS_API void sys_ms_register_map(SysMsMap *map);
SYS_API void sys_ms_unregister_map(SysMsMap *map);
SYS_API void sys_ms_collect(void);

SYS_API void sys_ms_block_prepend(SysHList *list);
SYS_API void sys_ms_block_remove(SysMsBlock* o);

SYS_API void sys_gc_setup(void);
SYS_API void sys_gc_teardown(void);

SYS_END_DECLS

#endif
