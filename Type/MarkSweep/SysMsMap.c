#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

void SysMsMap_cleanup(SysMsMap **addr) {
  sys_return_if_fail(addr != NULL);
  if(*addr == NULL) {

    sys_warning_N("Static variable not init or cleared, map address is: %p", addr);
    return;
  }

  sys_ms_map_free(*addr);
}

SysBool sys_ms_map_check(SysMsMap *o) {
  sys_return_val_if_fail(o != NULL, false);
  sys_return_val_if_fail(o->name != NULL, false);
  sys_return_val_if_fail(o->addr != NULL, false);

  return true;
}

SysMsMap *sys_ms_map_new_by_addr(void **frame_addr,
    void **addr,
    SysInt fbreg,
    const SysChar *name) {
  SysMsMap *o = ms_malloc0(sizeof(SysMsMap));

  o->fbreg = fbreg;
  o->addr = addr;
  o->name = name;
  o->frame_addr = frame_addr;
  sys_hlist_init((SysHList *)o);

  return o;
}

SysBool sys_ms_map_is_real(SysMsMap *map) {
  sys_return_val_if_fail(map != NULL, false);

  return *map->addr != SYS_MS_INIT_VALUE
    && *map->addr != NULL;
}

void sys_ms_map_free(SysMsMap *o) {
  sys_ms_map_remove(o);

  o->addr = NULL;
  ms_free(o);
}

