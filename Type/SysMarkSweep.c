#include <System/Type/SysMarkSweep.h>
#include <System/Type/SysObject.h>
#include <System/DataTypes/SysHList.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>

static SysMutex gc_lock;
/* SysMsMap */
static SysHList* g_map_list = NULL;
/* SysMsBlock */
static SysHList* g_block_list = NULL;

void sys_ms_lock(void) {

  sys_mutex_lock(&gc_lock);
}

void sys_ms_unlock(void) {

  sys_mutex_unlock(&gc_lock);
}

void sys_ms_setup(void) {
  g_block_list = NULL;
  sys_mutex_init(&gc_lock);
}

void sys_ms_teardown(void) {

  sys_mutex_clear(&gc_lock);
}

void _sys_ptr_clean(SysPointer *o) {
  *o = NULL;
}

void ms_block_mark(SysMsMap *o) {
  SysMsMap *mp;
  SysMsBlock *nb;
  SysHList *nlist = SYS_HLIST(o);

  sys_hlist_foreach(nlist, node) {
    mp = (SysMsMap *)node;
    if(*mp->addr == NULL) {
      continue;
    }

    nb = *mp->addr;
    nb->marked = true;
  }
}

void ms_block_sweep(SysMsBlock *o) {
  SysMsBlock *b;
  SysHList *node = SYS_HLIST(o);

  while(node) {
    b = (SysMsBlock *)node;
    if(b->bptr == NULL) {
      continue;
    }

    if(b->marked) {
      b->marked = false;
      continue;
    }

    node = node->next;

    sys_free(b->bptr);
    g_block_list = sys_hlist_delete_link(g_block_list, SYS_HLIST(b));
  }
}

SysPointer _sys_ms_alloc(SysInt type, SysSize size) {
  SysSize bsize;
  SysMsBlock* o;

  bsize = sizeof(SysMsBlock);
  o = sys_malloc0(bsize + size);
  o->type = type;
  o->bptr = (SysChar *)o + bsize;
  g_block_list = sys_hlist_prepend(g_block_list, SYS_HLIST(o));

  return o->bptr;
}

SysChar* sys_ms_alloc_str(const SysChar *str) {
  SysSize len;

  SysChar *nstr;

  len = strlen(str);
  nstr = _sys_ms_alloc(SYS_MS_STACK, len + 1);
  memcpy(nstr, str, len + 1);

  return nstr;
}

SysMsMap *sys_ms_map_alloc(void **addr) {
  SysMsMap *o = sys_new0(SysMsMap, 1);
  o->addr = addr;
  return o;
}

void sys_ms_register_var(void **addr) {
  SysMsMap *map;

  sys_mutex_lock(&gc_lock);

  map = sys_ms_map_alloc(addr);
  sys_ms_register_map(map);

  sys_mutex_unlock(&gc_lock);
}

void sys_ms_register_map(SysMsMap *map) {

  g_map_list = sys_hlist_prepend(g_map_list, SYS_HLIST(map));
}

void sys_ms_collect(void) {
  ms_block_mark((SysMsMap *)g_map_list);
  ms_block_sweep((SysMsBlock *)g_block_list);
}
