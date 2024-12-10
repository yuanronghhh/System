#include <System/Type/MarkSweep/SysMarkSweep.h>
#include <System/Type/MarkSweep/SysMsMap.h>
#include <System/Type/MarkSweep/SysMsBlock.h>
#include <System/Type/SysGcCommonPrivate.h>

#define NODE_TO_MS_BLOCK(o) ((SysMsBlock *)o)

/* SysMsMap */
static SYS_THREAD_LOCAL SysHQueue g_map_list;
/* SysMsBlock */
static SysHList* g_block_list = NULL;

/* allocator */
static SysMVTable allocator = {
  .malloc = sys_ms_block_malloc,
  .free = sys_ms_block_free,
  .realloc = sys_ms_block_realloc,
};

/* ms block */
void sys_ms_block_prepend(SysHList *list) {
  sys_return_if_fail(SYS_IS_HDATA(list));

  g_block_list = sys_hlist_prepend(g_block_list, list);
}

void sys_ms_block_remove(SysMsBlock* o) {
  g_block_list = sys_hlist_remove_link(g_block_list, SYS_HLIST(o));
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
      b = SYS_MS_BLOCK(*mp->addr);
      if(!SYS_IS_HDATA(b)) {

        sys_warning_N("pointer reference to invalid block: %p", mp->addr);
        continue;
      }

      if(b->type != SYS_MS_TRACK_AUTO) {
        continue;
      }

      b->status = SYS_MS_STATUS_MARKED;
    }
  }
}

static void ms_block_sweep(SysMsBlock *o) {
  SysMsBlock *b;
  SysPointer bptr;
  SysHList *node = SYS_HLIST(o);
  sys_return_if_fail(SYS_IS_HDATA(o));

  while(node) {
    b = NODE_TO_MS_BLOCK(node);
    bptr = SYS_MS_BLOCK_F_CAST(b);
    node = node->next;

    if(bptr == NULL) {
      continue;
    }

    if(b->type != SYS_MS_TRACK_AUTO) {
      continue;
    }

    if(b->status != SYS_MS_STATUS_MALLOCED) {
      continue;
    }

    sys_ms_block_remove(b);
  }
}

/* ms map */
void sys_ms_map_push_head(SysMsMap *o) {
  sys_return_if_fail(SYS_IS_HDATA(o));

  sys_hqueue_push_head(&g_map_list, SYS_HLIST(o));
}

void sys_ms_map_remove(SysMsMap *o) {
  sys_return_if_fail(SYS_IS_HDATA(o));

  sys_hqueue_unlink(&g_map_list, SYS_HLIST(o));
}

void sys_ms_unregister_map(SysMsMap *map) {
  sys_return_if_fail(SYS_IS_HDATA(map));

  sys_ms_map_free(map);
}

void sys_ms_register_map(SysMsMap *map) {
  sys_return_if_fail(SYS_IS_HDATA(map));

  sys_ms_map_push_head(map);
}

void sys_ms_unregister_var(SysMsMap **mapaddr) {
  SysMsBlock *b;
  SysMsMap *map = *mapaddr;
  void **addr = map->addr;

  if(!MS_IS_NULL_OR_INIT(addr)) {
    b = SYS_MS_BLOCK_B_CAST(*addr);
    b->type = SYS_MS_TRACK_AUTO;
  }

  sys_ms_map_free(map);
}

static void sys_ms_force_collect(void) {
  ms_block_mark(SYS_MS_MAP(sys_hqueue_peek_head(&g_map_list)));
  ms_block_sweep(NODE_TO_MS_BLOCK(g_block_list));
}

void sys_ms_collect(void) {
  sys_ms_force_collect();
}

void sys_ms_gc_setup(void) {
  sys_mem_set_vtable(&allocator);
  sys_hqueue_init(&g_map_list);
}

void sys_ms_gc_teardown(void) {
  SysPointer bptr;
  SysHList *node;
  SysMsBlock *b;

  node = SYS_HLIST(g_block_list);
  while(node) {
    b = NODE_TO_MS_BLOCK(node);
    node = node->next;
    bptr = SYS_MS_BLOCK_F_CAST(b);

    sys_info_N("memory leak block: %p", bptr);
  }

  sys_hqueue_clear(&g_map_list);
}

