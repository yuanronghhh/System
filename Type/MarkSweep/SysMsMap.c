#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

void SysMsMap_cleanup(SysMsMap **addr) {

  sys_ms_unregister_var(addr);
}

void sys_ms_map_construct(SysMsMap *o,
    void **addr,
    const SysChar *name,
    SysDestroyFunc destroy) {
  sys_hlist_init((SysHList *)o);
  o->destroy = destroy;
  o->addr = addr;
  o->name = name;
}

SysMsMap *sys_ms_map_new_by_addr(void **addr, const SysChar *name) {
  SysMsMap *o = ms_malloc0(sizeof(SysMsMap));

  sys_ms_map_construct(o, addr, name, free);

  return o;
}

void sys_ms_map_free(SysMsMap *o) {
  sys_ms_map_remove(o);

  o->addr = NULL;
  if (o->destroy) {

    o->destroy(o);
  }
}

