#include <System/Type/MarkSweep/SysMarkSweep.h>
#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/SysGcCommonPrivate.h>

#define NODE_TO_MS_BLOCK(o) ((SysMsBlock *)o)

/* SysMsMap */
static SYS_THREAD_LOCAL SysHList* g_map_list;
/* SysMsBlock */
static SysHList* g_block_list = NULL;
static SysMutex g_block_lock;

/* allocator */
static SysMVTable allocator = {
  .malloc = sys_ms_malloc,
  .free = sys_ms_free,
  .realloc = sys_ms_realloc,
};


/* stack */
/* ms block */
static void ms_reprepend(SysMsBlock *o) {
  SysHList *list = SYS_HLIST(o);
  sys_return_if_fail(list != NULL);

  g_block_list = sys_hlist_remove_link(g_block_list, list);
  g_block_list = sys_hlist_prepend(g_block_list, list);
}

static void ms_prepend(SysMsBlock *o) {
  SysHList *list = SYS_HLIST(o);
  sys_return_if_fail(list != NULL);

  g_block_list = sys_hlist_prepend(g_block_list, list);
}

static void ms_remove(SysMsBlock* o) {
  SysHList *list = SYS_HLIST(o);
  sys_return_if_fail(list != NULL);

  g_block_list = sys_hlist_remove_link(g_block_list, list);
}


SysPointer sys_ms_malloc(SysSize size) {
  SysMsBlock *o;

  o = sys_ms_block_malloc(size);
  ms_prepend(o);

  return SYS_MS_BLOCK_F_CAST(o);
}

void sys_ms_free(void* o) {
  SysMsBlock *self = SYS_MS_BLOCK(o);

  sys_mutex_lock(&g_block_lock);

  ms_remove(self);
  sys_ms_block_free(self);

  sys_mutex_unlock(&g_block_lock);
}

SysPointer sys_ms_realloc(SysPointer b, SysSize nsize) {
  SysMsBlock *o;

  o = sys_ms_pointer_realloc(b, nsize);
  ms_prepend(o);

  return SYS_MS_BLOCK_F_CAST(o);
}

static void ms_map_mark(SysHList *o) {
  SysMsMap *map;
  SysMsBlock *b;
  SysHList *node;

  node = SYS_HLIST(o);
  while(node) {
    map = SYS_MS_MAP(node);
    sys_assert(SYS_IS_MS_MAP(map));

    node = node->next;

    if(*map->addr == SYS_MS_INIT_VALUE) {
      continue;

    } else if(*map->addr == NULL) {
      sys_ms_map_free(map);

    } else {
      b = SYS_MS_BLOCK_B_CAST(*map->addr);
      if(!SYS_IS_HDATA(b)) {

        sys_warning_N("pointer reference to invalid block: %p, %s -> %p", map->addr, map->name, *map->addr);
        continue;
      }

      if(!sys_ms_block_need_mark(b)) {
        continue;
      }

      sys_ms_block_set_status(b, SYS_MS_STATUS_MARKED);
      ms_reprepend(b);
    }
  }
}

static void ms_block_sweep(SysHList *o) {
  SysMsBlock *b;
  SysHList *node = SYS_HLIST(o);

  while(node) {
    b = NODE_TO_MS_BLOCK(node);
    node = node->next;

    if(!sys_ms_block_need_sweep(b)) {
      sys_ms_block_set_status(b, SYS_MS_STATUS_MALLOCED);
      break;
    }

    ms_remove(b);
    sys_ms_block_free(b);
  }
}

/* ms map */
void sys_ms_map_prepend(SysMsMap *o) {
  sys_return_if_fail(SYS_IS_MS_MAP(o));

  g_map_list = sys_hlist_prepend(g_map_list, (SysHList *)o);
}

void sys_ms_map_remove(SysMsMap *o) {
  sys_return_if_fail(SYS_IS_MS_MAP(o));

  g_map_list = sys_hlist_remove_link(g_map_list, (SysHList *)o);
}

static void sys_ms_force_collect(void) {
  sys_mutex_lock(&g_block_lock);

  ms_map_mark(g_map_list);
  ms_block_sweep(g_block_list);

  sys_mutex_unlock(&g_block_lock);
}

void sys_ms_collect(void) {
  sys_ms_force_collect();
}


static SysMsMap *ms_get_first_map(void *ptr) {
  SysMsMap *map = NULL;
  SysHList *node;

  node = SYS_HLIST(g_map_list);
  while(node) {
    map = SYS_MS_MAP(node);
    node = node->next;

    if(!sys_ms_map_is_real(map)) {
      continue;
    }

    if(*map->addr == ptr) {

      return map;
    }
  }

  return NULL;
}

void sys_ms_gc_setup(void) {
  sys_mem_set_vtable(&allocator);
  sys_mutex_init(&g_block_lock);
}

void sys_ms_gc_teardown(void) {
  SysPointer bptr;
  SysHList *node;
  SysMsBlock *b;
  SysInt lcount = 0;
  SysMsMap *map;

  node = SYS_HLIST(g_block_list);
  while(node) {
    ++lcount;
    if(lcount > 20) { break; }

    b = NODE_TO_MS_BLOCK(node);
    node = node->next;
    bptr = SYS_MS_BLOCK_F_CAST(b);
    map = ms_get_first_map(bptr);
    sys_assert(map != NULL);

    sys_info_N("memory leak block: %s, %p", map->name, bptr);
  }

  g_map_list = NULL;
  sys_mutex_clear(&g_block_lock);
}

