#include <System/Type/SysMarkSweep.h>
#include <System/Type/SysObject.h>
#include <System/DataTypes/SysHQueue.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>

static SysMutex gc_lock;
/* SysMsMap */
static SysHQueue g_map_list;
/* SysMsBlock */
static SysHList* g_block_list = NULL;

static SysPointer ms_malloc0(SysSize size) {
  void *b = malloc(size);
  memset(b, 0, size);
  return b;
}

void sys_ms_lock(void) {

  sys_mutex_lock(&gc_lock);
}

void sys_ms_unlock(void) {

  sys_mutex_unlock(&gc_lock);
}

void sys_ms_setup(void) {
  g_block_list = NULL;

  sys_mem_set_vtable();

  sys_hqueue_init(&g_map_list);
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

  sys_hqueue_clear(&g_map_list);
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

        sys_warning_N("pointer reference to invalid block: %p", mp->addr);
        continue;
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

static void sys_ms_block_create(SysMsBlock *o, SysSize size) {
  SysHList *hlist;

  o->bptr = (SysChar *)o + sizeof(SysMsBlock);
  hlist = SYS_HLIST(o);
  sys_hlist_init(hlist);

  g_block_list = sys_hlist_prepend(g_block_list, hlist);
}

SysPointer sys_ms_block_alloc(SysSize size) {
  SysSize bsize = sizeof(SysMsBlock);
  SysMsBlock *o = ms_malloc0(bsize + size);

  sys_ms_block_create(o, size);

  return o->bptr;
}

SysChar* sys_ms_block_format(const SysChar *format, ...) {
  SysChar *str = NULL;

  va_list args;
  va_start(args, format);

  sys_vasprintf(&str, format, args);

  va_end(args);

  return str;
}

SysChar* sys_ms_block_strdup(const SysChar *str) {
  SysSize len;

  SysChar *nstr;

  len = strlen(str);
  nstr = sys_ms_block_alloc(len + 1);
  memcpy(nstr, str, len + 1);

  return nstr;
}

void sys_ms_map_init(SysMsMap *o) {
  sys_hlist_init(SYS_HLIST(o));
  o->destroy = NULL;
  o->addr = NULL;
}

void sys_ms_map_free(SysMsMap *o) {
  o->addr = NULL;
  sys_hqueue_unlink(&g_map_list, SYS_HLIST(o));

  if(o->destroy) {

    o->destroy(o);
  }
}

void sys_ms_map_push_head(SysMsMap *o) {

  sys_hqueue_push_head(&g_map_list, SYS_HLIST(o));
}

void sys_ms_unregister_map(void **addr, SysMsMap *map) {
  sys_mutex_lock(&gc_lock);

  if(map->addr != addr) {

    sys_warning_N("stack free failed: %p", addr);
    return;
  }

  sys_ms_map_free(map);
  sys_mutex_unlock(&gc_lock);
}

void sys_ms_unregister_var(void **addr) {
  SysMsMap *map = SYS_MS_MAP(sys_hqueue_peek_head(&g_map_list));
  sys_ms_unregister_map(addr, map);
}

void sys_ms_collect(void) {
  sys_mutex_lock(&gc_lock);

  ms_block_mark(SYS_MS_MAP(sys_hqueue_peek_head(&g_map_list)));
  ms_block_sweep(SYS_MS_BLOCK(g_block_list));

  sys_mutex_unlock(&gc_lock);
}
