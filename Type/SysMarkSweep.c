#include <System/Type/SysMarkSweep.h>
#include <System/Type/SysObject.h>
#include <System/DataTypes/SysHList.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>

struct _MsBlock {
  SysHList parent;
  SysPointer ptr;
  MsAddr addr;
};

static SysMutex gc_lock;
static MsBlock gc_slist;

void sys_ms_setup(void) {

  sys_mutex_init(&gc_lock);
  sys_hlist_init((SysHList *)&gc_slist);
}

void sys_ms_teardown(void) {

  sys_mutex_clear(&gc_lock);
}

void _sys_ptr_clean(SysPointer *o) {

  *o = NULL;
}


MsBlock *sys_ms_alloc(MsAddr *addr) {
  MsBlock *o = sys_new0(MsBlock, 1);

  o->addr = addr;

  return o;
}

void sys_ms_register_adress(MsAddr* addr) {
  sys_ms_alloc(addr);
}

void _sys_object_clean(SysObject **o) {

  sys_clear_pointer(o, _sys_object_unref);
}


void sys_gc_collect(void) {
}

