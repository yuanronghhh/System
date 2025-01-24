#include <System/Type/Ref/SysRefBlock.h>
#include <System/Platform/Common/SysRefCountPrivate.h>
#include <System/Type/Ref/SysRefBlock.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Utils/SysError.h>
#include <System/DataTypes/SysHCommon.h>

#define SYS_REF_BLOCK_CAST(o) _sys_hdata_b_cast(o, sizeof(SysRefBlock))
#define SYS_REF_BLOCK_F_CAST(o) _sys_hdata_f_cast(o, sizeof(SysRefBlock))

static SysRefHook sys_unref_debug_func = NULL;
static SysRefHook sys_ref_debug_func = NULL;
static SysRefHook sys_new_debug_func = NULL;

void sys_set_unref_hook(SysRefHook hook) {
  sys_unref_debug_func = hook;
}

void sys_set_ref_hook(SysRefHook hook) {
  sys_ref_debug_func = hook;
}

void sys_set_new_hook(SysRefHook hook) {
  sys_new_debug_func = hook;
}

SysPointer _sys_ref_block_cast_check(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK_CAST(o);
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p,%d", self->ref_count);
    return false;
  }

  return self;
}

SysBool sys_ref_block_create(SysRefBlock* self) {
  sys_ref_count_init(self);
  return true;
}

SysPointer sys_ref_block_malloc(SysSize size) {
  SysInt bsize = sizeof(SysRefBlock);
  SysRefBlock* o = sys_malloc0(bsize + size);

  sys_ref_block_create(o);

  if (sys_new_debug_func) {
    sys_new_debug_func(o,
        sys_ref_count_get(o));
  }

  return (SysChar *)o + bsize;
}

SysPointer sys_ref_block_realloc(SysRefBlock *b, SysSize size) {
  SysInt bsize = sizeof(SysRefBlock);
  SysRefBlock* o = sys_realloc(b, bsize + size);

  sys_ref_block_create(o);

  return (SysChar *)o + bsize;
}

void _sys_ref_block_destroy(SysRefBlock* self) {

  sys_ref_count_set(self, 0);
}

void sys_ref_block_unref(SysRefBlock *self) {
  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return;
  }


  if(sys_unref_debug_func) {

    sys_unref_debug_func(SYS_REF_BLOCK_F_CAST(self),
        sys_ref_count_get(self));
  }

  if(!sys_ref_count_dec(self)) {
    return;
  }

  sys_free(self);
}

SysPointer sys_ref_block_ref(SysRefBlock *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  if (!sys_ref_count_valid_check(self, MAX_REF_NODE)) {
    sys_warning_N("block ref check failed: %p", self);
    return NULL;
  }

  if(sys_ref_debug_func) {

    sys_ref_debug_func(self,
        sys_ref_count_get(self));
  }

  sys_ref_count_inc(self);

  return SYS_BLOCK(self);
}

void sys_ref_block_free(SysRefBlock *self) {
  sys_return_if_fail(self != NULL);

  sys_free(self);
}

SysBool sys_ref_block_valid_check(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK_CAST(o);

  return sys_ref_count_valid_check(self, MAX_REF_NODE);
}

SysBool sys_ref_block_check(SysPointer o) {
  SysRefBlock *self = SYS_REF_BLOCK_CAST(o);

  return sys_ref_count_check(self, MAX_REF_NODE);
}

void sys_ref_block_ref_init(SysRefBlock *self) {
  sys_return_if_fail(self != NULL);

  sys_ref_count_init(self);

  if (sys_new_debug_func) {
    sys_new_debug_func(self,
        sys_ref_count_get(self));
  }

}

void sys_ref_block_ref_inc(SysRefBlock *self) {
  sys_return_if_fail(self != NULL);
  if(sys_ref_debug_func) {

    sys_ref_debug_func(self,
        sys_ref_count_get(self));
  }

  sys_ref_count_inc(self);
}

SysRef sys_ref_block_ref_get(SysRefBlock *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return sys_ref_count_get(self);
}

SysBool sys_ref_block_ref_dec(SysRefBlock *self) {
  sys_return_val_if_fail(self != NULL, -1);
  if(sys_unref_debug_func) {

    sys_unref_debug_func(SYS_REF_BLOCK_F_CAST(self),
        sys_ref_count_get(self));
  }

  return sys_ref_count_dec(self);
}

SysBool sys_ref_block_ref_cmp(SysRefBlock *self, SysRef n) {
  sys_return_val_if_fail(self != NULL, -1);

  return sys_ref_count_cmp(self, n);
}

void sys_ref_block_ref_set(SysRefBlock *self, SysRef n) {
  sys_return_if_fail(self != NULL);

  sys_ref_count_set(self, n);
}
