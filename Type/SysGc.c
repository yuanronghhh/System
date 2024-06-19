#include <System/Type/SysGc.h>
#include <System/Type/SysType.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>
#include <System/DataTypes/SysHashTable.h>

static SysRefHook sgc_block_unref_debug_func = NULL;
static SysRefHook sgc_block_ref_debug_func = NULL;
static SysRefHook sgc_block_new_debug_func = NULL;

static SysMutex gc_lock;
static SysHashTable *g_root_heap;

void sgc_setup(void) {
  sys_mutex_init(&gc_lock);

  g_root_heap = sys_hash_table_new_full(sys_direct_hash,
      (SysEqualFunc)sys_direct_equal,
      NULL,
      (SysDestroyFunc)sys_free);
}

void sgc_teardown(void) {
  sys_clear_pointer(&g_root_heap, sys_hash_table_unref);
  sys_mutex_clear(&gc_lock);
}

void sgc_run(SgcCollector *gc) {
}

void sgc_stop(SgcCollector* gc) {
}

SgcBlock* sgc_block_malloc0(SysType type, SysSize size) {
  sys_return_val_if_fail(size > sizeof(SgcBlock), NULL);
  SgcBlock* o =  sys_malloc0_N(size);

  o->type = type;
  sys_ref_count_init(o);

  return o;
}

SysPointer _sgc_type_new(SysType type, SysSize count) {
  SysTypeInstance *instance;
  SysSize size, priv_size;

  if(!sys_type_instance_get_size(type, &size, &priv_size)) {
    return NULL;
  }
  instance = sys_malloc0_N(size * count);;

  return instance;
}

void sgc_type_free(SysPointer ptr) {

  sys_free(ptr);
}

SysPointer sgc_malloc0(SysSize size) {
  sys_return_val_if_fail(size > 0, NULL);

  return sys_malloc0(size);
}

void sgc_free(SysPointer ptr) {

  sys_free(ptr);
}

SgcArea *sgc_area_new(SysPointer *addr, SysInt isstack, SgcBlock *block) {
  SgcArea *area;

  area = sys_new0_N(SgcArea, 1);
  area->last_addr = addr;
  area->block = block;
  area->isstack = isstack;

  return area;
}

void sgc_set(SysPointer *addr, SysInt isstack, SgcBlock *block) {
  SysHashTable *block_heap;
  SgcArea *area;

  block_heap = (SysHashTable *)sys_hash_table_lookup(g_root_heap,
      (SysPointer)block->type);
  if(block_heap == NULL) {
    block_heap = sys_hash_table_new_full(sys_direct_hash,
        (SysEqualFunc)sys_direct_equal,
        NULL,
        (SysDestroyFunc)sys_free);
  }

  area = sgc_area_new(addr, isstack, block);
  sys_hash_table_insert(block_heap, addr, area);
}

void sgc_save(SysPointer *ptr, SysInt isstack) {
}

void sgc_unsave(SysPointer *ptr) {
}

void sgc_block_set_unref_hook(SysRefHook hook) {
  sgc_block_unref_debug_func = hook;
}

void sgc_block_set_ref_hook(SysRefHook hook) {
  sgc_block_ref_debug_func = hook;
}

void sgc_block_set_new_hook(SysRefHook hook) {
  sgc_block_new_debug_func = hook;
}

SysPointer _sys_block_cast_check(SgcBlock *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!SYS_REF_VALID_CHECK(self, MAX_REF_NODE)) {
    sys_warning_N("object ref check failed: %p", self);
    return false;
  }

  return self;
}


static SysBool block_create(SgcBlock* self, SysType type, ...) {
  SysTypeNode *node;

  node = sys_type_node(type);
  if (!sys_type_instance_create((SysTypeInstance*)self, type)) {
    return false;
  }

  sgc_block_ref_init(self);

  if (sgc_block_new_debug_func) {

    sgc_block_new_debug_func(self,
        sys_type_node_name(node), 
        sgc_block_ref_get(self));
  }

  return true;
}

SysBool _sgc_block_create(SgcBlock *o,
    SysType type, 
    const SysChar * first, 
    ...) {

  sys_return_val_if_fail(o != NULL, false);
  o->type = type;

  return block_create (o, type, NULL);
}

void* sgc_block_new(SysType type, const SysChar * first, ...) {
  SgcBlock *o;

  o = (SgcBlock *)sys_type_instance_new(type, 1);
  if (!block_create(o, type)) {
    sys_type_instance_free((SysTypeInstance *)o);
    return NULL;
  }

  return o;
}

SysBool block_destroy(SgcBlock* self) {
    sys_return_val_if_fail(self != NULL, false);

    if (!SYS_REF_CHECK(self, MAX_REF_NODE)) {
        sys_warning_N("block ref check failed: %p", self);
        return false;
    }

    if (sgc_block_unref_debug_func) {
        SysType type;
        SysTypeNode* node;

        type = sys_type_from_instance(self);
        node = sys_type_node(type);

        sgc_block_unref_debug_func(self,
            sys_type_node_name(node),
            sys_atomic_int_get(&self->ref_count));
    }

    if (!sgc_block_ref_dec(self)) {
        return false;
    }

    return true;
}

SysBool _sgc_block_destroy(SgcBlock* self) {

    return block_destroy(self);
}

SysBool _sgc_block_unref(SgcBlock* self) {
  if (!SYS_REF_VALID_CHECK(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return false;
  }

  if(!sgc_block_ref_dec(self)) {
    return false;
  }

  sgc_block_free(self);

  return true;
}

SysPointer _sgc_block_ref(SgcBlock* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!SYS_REF_VALID_CHECK(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return false;
  }

  if (sgc_block_ref_debug_func) {
    SysType type = sys_type_from_sgc_block(self);
    SysTypeNode *node = sys_type_node(type);

    sgc_block_ref_debug_func(self,
        sys_type_node_name(node), 
        sys_atomic_int_get(&self->ref_count));
  }

  sgc_block_ref_inc(self);

  return (SysPointer)self;
}

void _sgc_block_free(SgcBlock* self) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(sgc_block_ref_check(self, MAX_REF_NODE));

  sys_free_N(self);
}
