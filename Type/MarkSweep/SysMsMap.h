#ifndef __SYS_MS_MAP_H__
#define __SYS_MS_MAP_H__

#include <System/Type/SysGcCommon.h>

SYS_BEGIN_DECLS

#define SYS_MS_MAP(o) ((SysMsMap *)o)
#define SYS_IS_MS_MAP(o) ((SysMsMap *)sys_ms_map_check(o))

struct _SysMsMap {
  SysHList unowned;

  /* <private> */
  void** frame_addr;
  void** addr;
  SysInt fbreg;
  const SysChar *name;
};

SYS_API SysMsMap *sys_ms_map_new(void);
SYS_API void sys_ms_map_free(SysMsMap *o);
SYS_API SysBool sys_ms_map_is_real(SysMsMap *map);
SYS_API SysMsMap *sys_ms_map_new_by_addr(void **frame_addr,
    void **addr, 
    SysInt fbreg,
    const SysChar *name);
SYS_API SysBool sys_ms_map_check(SysMsMap *o);

SYS_END_DECLS

#endif
