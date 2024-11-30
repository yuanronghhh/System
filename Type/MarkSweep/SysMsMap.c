#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

void SysMsMap_cleanup(SysMsMap **addr) {

  sys_ms_unregister_var(addr);
}

void sys_ms_map_construct(SysMsMap *o, void **addr, SysDestroyFunc destroy) {
  sys_hlist_init(SYS_HLIST(o));
  o->destroy = destroy;
  o->addr = addr;
}

SysMsMap *sys_ms_map_new_by_addr(void **addr) {
  SysMsMap *o = ms_malloc0(sizeof(SysMsMap));

  sys_ms_map_construct(o, addr, sys_free);

  return o;
}

void sys_ms_map_free(SysMsMap *o) {
  if(o->destroy) {

    o->destroy(o);
  }

  o->addr = NULL;
  sys_ms_map_remove(o);
}

