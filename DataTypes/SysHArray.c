#include <System/DataTypes/SysHArray.h>
#include <System/Platform/Common/SysRefCount.h>
#include <System/DataTypes/SysHashTable.h>

#define MIN_ARRAY_SIZE  16
static void harray_maybe_expand(SysHArray *self, SysUInt len);


static SysHArray * ptr_harray_new(SysUInt reserved_size,
    SysDestroyFunc element_free_func) {
    SysHArray *array;

    array = sys_new0(SysHArray, 1);
    array->pdata = NULL;
    array->len = 0;
    array->alloc = 0;
    array->element_free_func = element_free_func;
    sys_ref_count_init(array);

    if (reserved_size != 0)
        harray_maybe_expand(array, reserved_size);

    return array;
}

SysHArray* sys_harray_new(void) {
  return ptr_harray_new(0, NULL);
}

static SysPointer harray_remove_index(SysHArray* array,
  SysUInt      index_,
  SysBool   fast,
  SysBool   free_element) {
  SysPointer result;

  sys_return_val_if_fail(array, NULL);
  sys_return_val_if_fail(array->len == 0
      || (array->len != 0 && array->pdata != NULL), NULL);

  sys_return_val_if_fail(index_ < array->len, NULL);

  result = array->pdata[index_];

  if (array->element_free_func != NULL && free_element)
    array->element_free_func(array->pdata[index_]);

  if (index_ != array->len - 1 && !fast)
    memmove(array->pdata + index_, array->pdata + index_ + 1,
      sizeof(SysPointer) * (array->len - index_ - 1));
  else if (index_ != array->len - 1)
    array->pdata[index_] = array->pdata[array->len - 1];

  array->len -= 1;

  return result;
}

SysBool sys_harray_remove(SysHArray *array,
    SysPointer   data) {
    SysUInt i;

    sys_return_val_if_fail(array, false);
    sys_return_val_if_fail(array->len == 0 || (array->len != 0 && array->pdata != NULL), false);

    for (i = 0; i < array->len; i += 1)
    {
        if (array->pdata[i] == data)
        {
            sys_harray_remove_index(array, i);
            return true;
        }
    }

    return false;
}


SysPointer sys_harray_remove_index_fast(SysHArray *array,
    SysUInt      index_) {

    return harray_remove_index(array, index_, true, true);
}

SysPointer sys_harray_remove_index(SysHArray *array,
    SysUInt      index_) {

  return harray_remove_index(array, index_, false, true);
}

SysPointer sys_harray_steal_index(SysHArray* array, SysUInt index_) {
  return harray_remove_index(array, index_, false, false);
}

SysHArray* sys_harray_new_with_free_func(SysDestroyFunc element_free_func) {
  return ptr_harray_new(0, element_free_func);
}

void sys_harray_destroy(SysHArray* self) {
  SysPointer* sp;

  if (self->element_free_func) {
    for (SysUInt i = 0; i < self->len; ++i) {
      self->element_free_func(self->pdata[i]);
    }

    sp = sys_steal_pointer(&self->pdata);

    if(sp != NULL) {
      sys_free(sp);
    }
  }
}

void sys_harray_free(SysHArray* self, SysBool free_segment) {
  if (free_segment) {

    sys_harray_destroy(self);
  } else {

    self->len = 0;
    self->pdata = NULL;
    self->alloc = 0;
  }
  sys_free(self);
}

void sys_harray_unref(SysHArray* self) {
  sys_return_if_fail(self != NULL);

  if (sys_ref_count_dec(self)) {

    sys_harray_free(self, true);
  }
}

SysHArray* sys_harray_ref(SysHArray *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  sys_ref_count_inc(self);

  return self;
}

void sys_harray_copy(SysHArray* dst,
    SysHArray* src, 
    SysCopyFunc elem_copy, 
    SysPointer copy_user_data) {
  for (SysUInt i = 0; i < src->len; i++) {
    sys_harray_add(dst, elem_copy(src->pdata[i], copy_user_data));
  }
}

static void harray_maybe_expand(SysHArray *self, SysUInt len) {
  if ((UINT_MAX - self->len) < len) {
    sys_error_N("adding %u to self would overflow", len);
  }

  if ((self->len + len) > self->alloc) {
    self->alloc = sys_nearest_pow(self->len + len);
    self->alloc = max(self->alloc, MIN_ARRAY_SIZE);
    self->pdata = sys_realloc(self->pdata, sizeof(SysPointer) * self->alloc);
  }
}

void sys_harray_init(SysHArray *self) {
  sys_harray_init_full(self, 0, NULL);
}

void sys_harray_init_full(SysHArray* self,
    SysUInt reserved_size, 
    SysDestroyFunc element_free_func) {
  sys_return_if_fail(self != NULL);

  self->pdata = NULL;
  self->len = 0;
  self->alloc = 0;
  self->element_free_func = element_free_func;
  sys_ref_count_init(self);

  if (reserved_size != 0)
    harray_maybe_expand(self, reserved_size);
}

void sys_harray_init_with_free_func(SysHArray* self,
    SysDestroyFunc element_free_func) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(element_free_func != NULL);

  sys_harray_init_full(self, 0, element_free_func);
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

SysPointer sys_harray_find(SysHArray *self,
    SysEqualFunc     equal_func,
    const SysPointer needle,
    SysInt *index_) {
  SysUInt i;

  sys_return_val_if_fail(self != NULL, NULL);
  sys_return_val_if_fail(equal_func != NULL, NULL);

  for (i = 0; i < self->len; i++) {
    if (equal_func(self->pdata[i], needle)) {
      *index_ = i;
      return self->pdata[i];
    }
  }

  return NULL;
}
