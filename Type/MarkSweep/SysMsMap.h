#ifndef __SYS_MS_MAP_H__
#define __SYS_MS_MAP_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_MS_MAP(o) ((SysMsMap *)o)
#define SYS_IS_MS_MAP(o) ((SysMsMap *)sys_ms_map_check(o))

struct _SysMsMap {
  SysHList unowned;
  /* <private> */
  void** addr;
  const SysChar *name;
  SysShort type;
};

SYS_API void sys_ms_map_push_head(SysMsMap *o);
SYS_API void sys_ms_map_push_tail(SysMsMap *o);
SYS_API SysMsMap *sys_ms_map_new(void);
SYS_API void sys_ms_map_construct(SysMsMap *o,
    void **addr,
    const SysChar *name,
    SysShort type);
SYS_API void sys_ms_map_free(SysMsMap *o);
SYS_API SysBool sys_ms_map_is_real(SysMsMap *map);
SYS_API SysMsMap *sys_ms_map_new_by_addr(void **addr, const SysChar *name);
SYS_API void SysMsMap_cleanup(SysMsMap **addr);
SYS_API SysBool sys_ms_map_check(SysMsMap *o);

SYS_END_DECLS

#endif
