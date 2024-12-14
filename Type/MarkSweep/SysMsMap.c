#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMarkSweep.h>

void SysMsMap_cleanup(SysMsMap **addr) {
  sys_return_if_fail(addr != NULL);
  if(*addr == NULL) {

    sys_warning_N("Static variable must init, map address is: %p", addr);
    return;
  }

  sys_ms_map_free(*addr);
}

void sys_ms_map_construct(SysMsMap *o,
    void **addr,
    const SysChar *name,
    SysShort type) {

  o->addr = addr;
  o->name = name;
  o->type = type;

  sys_hlist_init((SysHList *)o);
}

SysBool sys_ms_map_check(SysMsMap *o) {
  sys_return_val_if_fail(o != NULL, false);
  sys_return_val_if_fail(o->name != NULL, false);
  sys_return_val_if_fail(o->addr != NULL, false);
  sys_return_val_if_fail(o->type >= 0, false);

  return true;
}

SysMsMap *sys_ms_map_new_by_addr(void **addr, const SysChar *name) {
  SysMsMap *o = ms_malloc0(sizeof(SysMsMap));

  sys_ms_map_construct(o, addr, name, 1);

  return o;
}

void sys_ms_map_free(SysMsMap *o) {
  sys_ms_map_remove(o);

  o->addr = NULL;
  if (o->type) {

    ms_free(o);
  }
}

