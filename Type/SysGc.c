#include <System/Type/SysGc.h>
#include <System/Type/SysObject.h>
#include <System/Type/SysType.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>
#include <System/DataTypes/SysHashTable.h>

static SysRefHook sgc_block_unref_debug_func = NULL;
static SysRefHook sgc_block_ref_debug_func = NULL;
static SysRefHook sgc_block_new_debug_func = NULL;

void sgc_setup(void) {
}

void sgc_teardown(void) {
}

SgcBlock* sgc_block_malloc0(SysType type, SysSize size) {
  sys_return_val_if_fail(size > sizeof(SgcBlock), NULL);
  SgcBlock* o =  sys_malloc0_N(size);

  o->type = type;
  sgc_block_ref_init(o);

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

  if (!sgc_block_ref_check(self, MAX_REF_NODE)) {
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

SysBool _sgc_block_destroy_check(SgcBlock* self) {
  sys_return_val_if_fail(self != NULL, false);

  if (!sgc_block_ref_valid_check(self, MAX_REF_NODE)) {
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
        sgc_block_ref_get(self));
  }

  return sgc_block_ref_dec(self);
}

void _sgc_block_destroy(SgcBlock* self) {

  sys_ref_count_set(self, 0);
}

void _sgc_block_unref(SgcBlock* self) {
  if (!sgc_block_ref_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return;
  }
  if(!sgc_block_ref_dec(self)) {
    return;
  }

  sgc_block_free(self);
}

SysPointer _sgc_block_ref(SgcBlock* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sgc_block_ref_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return NULL;
  }

  if (sgc_block_ref_debug_func) {
    SysType type = sys_type_from_sgc_block(self);
    SysTypeNode *node = sys_type_node(type);

    sgc_block_ref_debug_func(self,
        sys_type_node_name(node),
        sgc_block_ref_get(self));
  }

  sgc_block_ref_inc(self);

  return (SysPointer)self;
}

void _sgc_block_free(SgcBlock* self) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(sgc_block_ref_check(self, MAX_REF_NODE));

  sys_free_N(self);
}
