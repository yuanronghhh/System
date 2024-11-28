#include <System/Type/SysMarkSweep.h>
#include <System/Type/SysObject.h>
#include <System/DataTypes/SysHQueue.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>

#define ms_realloc(o, size) realloc(o, size)
#define ms_malloc(size) malloc(size)
#define ms_free(o) free(o)
#define MS_BSIZE sizeof(SysMsBlock)
#define ms_b_cast(o) _sys_hdata_b_cast(o, MS_BSIZE)
#define ms_f_cast(o) _sys_hdata_f_cast(o, MS_BSIZE)

static SysMutex gc_lock;
/* SysMsMap */
static SysHQueue g_map_list;
/* SysMsBlock */
static SysHList* g_block_list = NULL;

static SysMVTable allocator = {
  .malloc = sys_ms_block_malloc,
  .free = sys_ms_block_free,
  .realloc = sys_ms_block_realloc,
};

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

/* ms block */
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
      b =  ms_b_cast(*mp->addr);
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
  SysPointer bptr;
  SysHList *node = SYS_HLIST(o);

  while(node) {
    b = SYS_MS_BLOCK(node);
    node = node->next;
    bptr = ms_f_cast(b);

    if(bptr == NULL) {
      continue;
    }

    if(b->marked) {
      b->marked = false;
      continue;
    }

    sys_ms_block_remove(b);
    ms_free(b);
  }
}

void sys_ms_block_free(SysPointer o) {
  sys_return_if_fail(o != NULL);

  SysMsBlock *b = ms_b_cast(o);
  sys_return_if_fail(SYS_IS_HDATA(b));

  sys_ms_block_remove(b);
  ms_free(b);
}

void sys_ms_block_remove(SysMsBlock* o) {

  g_block_list = sys_hlist_remove_link(g_block_list, SYS_HLIST(o));
}

static void sys_ms_block_create(SysMsBlock *o) {
  SysHList *ms_list;

  ms_list = SYS_HLIST(o);
  sys_hlist_init(ms_list);

  g_block_list = sys_hlist_prepend(g_block_list, ms_list);
}

SysPointer sys_ms_block_realloc(SysPointer b, SysSize nsize) {
  SysMsBlock *o = ms_realloc(b, MS_BSIZE + nsize);

  sys_ms_block_create(o);

  return ms_f_cast(o);
}

SysPointer sys_ms_block_malloc(SysSize size) {
  SysMsBlock *o = ms_malloc0(MS_BSIZE + size);

  sys_ms_block_create(o);

  return ms_f_cast(o);
}

/* ms map */
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

void sys_ms_setup(void) {
  g_block_list = NULL;

  sys_mem_set_vtable(&allocator);
  sys_hqueue_init(&g_map_list);
  sys_mutex_init(&gc_lock);
}

void sys_ms_teardown(void) {
  SysMsBlock *b;
  SysHList *node;
  SysPointer bptr;

  node = SYS_HLIST(g_block_list);
  while(node) {
    b = SYS_MS_BLOCK(node);
    node = node->next;
    bptr = ms_f_cast(b);

    sys_info_N("memory leak block: %p", bptr);
  }

  sys_hqueue_clear(&g_map_list);
  sys_mutex_clear(&gc_lock);
}

