#include <System/Type/SysBlock.h>
#include <System/Type/SysObject.h>
#include <System/Type/SysType.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysThread.h>
#include <System/DataTypes/SysHashTable.h>

static SysRefHook sys_block_unref_debug_func = NULL;
static SysRefHook sys_block_ref_debug_func = NULL;
static SysRefHook sys_block_new_debug_func = NULL;

void sys_block_set_unref_hook(SysRefHook hook) {
  sys_block_unref_debug_func = hook;
}

void sys_block_set_ref_hook(SysRefHook hook) {
  sys_block_ref_debug_func = hook;
}

void sys_block_set_new_hook(SysRefHook hook) {
  sys_block_new_debug_func = hook;
}

SysPointer _sys_block_cast_check(SysBlock *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_check(self, MAX_REF_NODE)) {
    sys_warning_N("object ref check failed: %p", self);
    return false;
  }

  return self;
}

static SysBool sys_block_create(SysBlock* self, SysType type) {
  self->type = type;

  sys_ref_count_init(self);

  return true;
}

void sys_block_free(SysBlock* b) {
  sys_assert(b->ref_count == 0);

  sys_free(b);
}

SysPointer sys_block_new(SysType tp, SysSize size) {
  SysBlock* o =  sys_malloc0(size);

  sys_block_create(o, tp);

  return o;
}

SysBool _sys_block_destroy_check(SysBlock* self) {
  sys_return_val_if_fail(self != NULL, false);

  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return false;
  }

  if (sys_block_unref_debug_func) {
    SysType type;
    SysTypeNode* node;

    type = sys_type_from_instance(self);
    node = sys_type_node(type);

    sys_block_unref_debug_func(self,
        sys_type_node_name(node),
        sys_ref_count_get(self));
  }

  return sys_ref_count_dec(self);
}

void _sys_block_destroy(SysBlock* self) {

  self->type = 0;
  sys_ref_count_set(self, 0);
}

void _sys_block_unref(SysBlock* self) {
  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return;
  }

  if(!sys_ref_count_dec(self)) {
    return;
  }

  sys_block_free(self);
}

SysPointer _sys_block_ref(SysBlock* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return NULL;
  }

  if (sys_block_ref_debug_func) {
    SysType type = sys_type_from_sys_block(self);
    SysTypeNode *node = sys_type_node(type);

    sys_block_ref_debug_func(self,
        sys_type_node_name(node),
        sys_ref_count_get(self));
  }

  sys_ref_count_inc(self);

  return (SysPointer)self;
}

void _sys_block_free(SysBlock* self) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(sys_ref_count_check(self, MAX_REF_NODE));

  sys_free(self);
}
