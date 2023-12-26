#include <System/DataTypes/SysHArray.h>

#define MIN_ARRAY_SIZE  16

static void harray_maybe_expand(SysHArray *self, SysUInt len) {
  if ((UINT_MAX - self->len) < len) {
    sys_error_N("adding %u to self would overflow", len);
  }

  if ((self->len + len) > self->alloc) {
    self->alloc = sys_nearest_pow(self->len + len);
    self->alloc = max(self->alloc, MIN_ARRAY_SIZE);
    self->pdata = sys_realloc_N(self->pdata, sizeof(SysPointer) * self->alloc);
  }
}

SysBool sys_harray_init(SysHArray *self) {
  return sys_harray_init_full(self, 0, NULL);
}

SysBool sys_harray_init_full(SysHArray* self, SysUInt reserved_size, SysDestroyFunc element_free_func) {
  sys_return_val_if_fail(self != NULL, false);

  self->pdata = NULL;
  self->len = 0;
  self->alloc = 0;
  self->element_free_func = element_free_func;

  if (reserved_size != 0)
    harray_maybe_expand(self, reserved_size);

  return true;
}

SysBool sys_harray_init_with_free_func(SysHArray* self, SysDestroyFunc element_free_func) {
  sys_return_val_if_fail(self != NULL, false);
  sys_return_val_if_fail(element_free_func != NULL, false);

  return sys_harray_init_full(self, 0, element_free_func);
}

void sys_harray_add(SysHArray *self, SysPointer data) {
  sys_return_if_fail(self);
  sys_return_if_fail(self->len == 0 || (self->len != 0 && self->pdata != NULL));

  harray_maybe_expand(self, 1);

  self->pdata[self->len++] = data;
}

void sys_harray_insert(SysHArray *self, SysInt index_, SysPointer data) {

  sys_return_if_fail(self);
  sys_return_if_fail(index_ >= -1);
  sys_return_if_fail(index_ <= (SysInt)self->len);

  harray_maybe_expand(self, 1);

  if (index_ < 0)
    index_ = self->len;

  if ((SysUInt)index_ < self->len)
    memmove(&(self->pdata[index_ + 1]),
        &(self->pdata[index_]),
        (self->len - index_) * sizeof(SysPointer));

  self->len++;
  self->pdata[index_] = data;
}
