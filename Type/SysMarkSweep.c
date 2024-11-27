#include <System/Type/SysMarkSweep.h>
#include <System/Type/SysObject.h>
#include <System/DataTypes/SysHQueue.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>

#define SYS_MS_MAP(o) ((SysMsMap *)o)
#define SYS_MS_BLOCK(o) ((SysMsBlock *)o)

static SysMutex gc_lock;
/* SysMsMap */
static SysHQueue* g_map_list = NULL;
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
  SysMsBlock *b;
  SysHList *node;

  node = SYS_HLIST(g_block_list);
  while(node) {
    b = SYS_MS_BLOCK(node);
    node = node->next;

    sys_info_N("memory leak block: %p", b->bptr);
  }

  sys_mutex_clear(&gc_lock);
}

static void ms_block_remove (SysMsBlock *o) {

  g_block_list = sys_hlist_delete_link(g_block_list, SYS_HLIST(o));
}

static void ms_block_mark(SysMsMap *o) {
  SysMsMap *mp;
  SysMsBlock *b;
  SysHList *node = SYS_HLIST(o);

  while(node) {
    mp = SYS_MS_MAP(node);
    node = node->next;

    if(*mp->addr == SYS_MS_INIT_VALUE) {
      continue;

    } else if(*mp->addr == NULL) {
      sys_ms_map_free(mp);

    } else {
      b = (SysMsBlock *)(((SysChar *)*mp->addr) - sizeof(SysMsBlock));
      if(!SYS_IS_HDATA(b)) {

        sys_error_N("pointer reference to invalid block: %lx", mp->addr);
      }

      b->marked = true;
    }
  }
}

static void ms_block_sweep(SysMsBlock *o) {
  SysMsBlock *b;
  SysHList *node = SYS_HLIST(o);

  while(node) {
    b = SYS_MS_BLOCK(node);
    node = node->next;

    if(b->bptr == NULL) {
      continue;
    }

    if(b->marked) {
      b->marked = false;
      continue;
    }

    sys_ms_block_free(b);
  }
}

void sys_ms_block_free(SysMsBlock* o) {
  o->bptr = NULL;
  ms_block_remove(o);
}

void sys_ms_block_create(SysMsBlock *o, SysSize size) {
  SysHList *hlist;

  o->bptr = (SysChar *)o + sizeof(SysMsBlock);
  hlist = SYS_HLIST(o);
  sys_hlist_init(hlist);

  g_block_list = sys_hlist_prepend(g_block_list, hlist);
}

SysPointer sys_ms_block_alloc(SysSize size) {
  SysSize bsize = sizeof(SysMsBlock);
  SysMsBlock *o = sys_malloc0(bsize + size);

  sys_ms_block_create(o, size);

  return o->bptr;
}

SysChar* sys_ms_block_alloc_str(const SysChar *str) {
  SysSize len;

  SysChar *nstr;

  len = strlen(str);
  nstr = sys_ms_block_alloc(len + 1);
  memcpy(nstr, str, len + 1);

  return nstr;
}

void sys_ms_map_free(SysMsMap *o) {
  o->addr = NULL;
  sys_hqueue_unlink(g_map_list, SYS_HLIST(o));

  if(o->destroy) {

    o->destroy(o);
  }
}

SysMsMap *sys_ms_map_alloc(void **addr) {
  SysMsMap *o;
  SysHList *hlist;

  o = sys_new0(SysMsMap, 1);
  o->addr = addr;

  hlist = SYS_HLIST(o);
  sys_hlist_init(hlist);

  sys_hqueue_push_head_link(g_map_list, hlist);

  return o;
}

void sys_ms_map_push_head(SysMsMap *o) {

  sys_hqueue_push_head(g_map_list, SYS_HLIST(o));
}

void sys_ms_unregister_var(void **addr) {
  sys_mutex_lock(&gc_lock);

  SysMsMap *map = SYS_MS_MAP(g_map_list);
  if(map->addr != addr) {

    sys_warning_N("stack free failed: %lx", addr);
    return;
  }

  sys_ms_map_free(map);
  sys_mutex_unlock(&gc_lock);
}

void sys_ms_register_var(void **addr) {
  sys_mutex_lock(&gc_lock);

  sys_ms_map_alloc(addr);

  sys_mutex_unlock(&gc_lock);
}

void sys_ms_collect(void) {
  sys_mutex_lock(&gc_lock);

  ms_block_mark(SYS_MS_MAP(g_map_list));
  ms_block_sweep(SYS_MS_BLOCK(g_block_list));

  sys_mutex_unlock(&gc_lock);
}
