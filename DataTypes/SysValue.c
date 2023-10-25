#include <System/DataTypes/SysValue.h>
#include <System/Utils/SysString.h>

struct _SysValue {
  SYS_VALUE_ENUM data_type;

  union {
    SysPointer v_pointer;
    SysBool v_bool;
    SysDouble v_double;
    SysInt v_int;
    SysChar* v_string;
    SysPtrArray *v_array;
  } v;

  SysRef ref_count;
};

SysInt sys_value_get_data_type(SysValue *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->data_type;
}

void sys_value_set_v_pointer(SysValue *self, SysPointer v_pointer) {
  sys_return_if_fail(self != NULL);

  self->data_type = SYS_VALUE_POINTER;
  self->v.v_pointer = v_pointer;
}

SysPointer sys_value_get_v_pointer(SysValue *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  return self->v.v_pointer;
}

SysValue* sys_value_new(void) {
  SysValue *o = sys_new0_N(SysValue, 1);

  sys_ref_count_init(o);

  return o;
}

SysValue *sys_value_copy(SysValue *oself) {
  sys_return_val_if_fail(oself != NULL, NULL);

  SysValue *nself = sys_value_new();

  switch (oself->data_type) {
    case SYS_VALUE_STRING:
      sys_value_set_v_string(nself, oself->v.v_string);
      break;
    case SYS_VALUE_INT:
      sys_value_set_v_int(nself, oself->v.v_int);
      break;
    case SYS_VALUE_DOUBLE:
      sys_value_set_v_double(nself, oself->v.v_double);
      break;
    case SYS_VALUE_NULL:
      sys_value_set_v_null(nself);
      break;
    case SYS_VALUE_POINTER:
      sys_value_set_v_pointer(nself, oself->v.v_pointer);
      break;
    default:
      break;
  }

  return nself;
}

void sys_value_set_v_double(SysValue *self, SysDouble v_double) {
  sys_return_if_fail(self != NULL);

  self->data_type = SYS_VALUE_DOUBLE;
  self->v.v_double = v_double;
}

SysDouble sys_value_get_v_double(SysValue *self) {
  sys_return_val_if_fail(self != NULL, 0);

  return self->v.v_double;
}

void sys_value_set_v_int(SysValue *self, SysInt v_int) {
  sys_return_if_fail(self != NULL);

  self->data_type = SYS_VALUE_INT;
  self->v.v_int = v_int;
}

SysInt sys_value_get_v_int(SysValue *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->v.v_int;
}

void sys_value_set_v_null(SysValue *self) {
  sys_return_if_fail(self != NULL);

  self->data_type = SYS_VALUE_NULL;
  self->v.v_pointer = NULL;
}

void sys_value_set_v_string(SysValue *self, const SysChar * v_string) {
  sys_return_if_fail(self != NULL);

  if(self->v.v_string) {
    sys_clear_pointer(&self->v.v_string, sys_free);
  }

  self->v.v_string = sys_strdup(v_string);
}

const SysChar * sys_value_get_v_string(SysValue *self) {
  sys_return_val_if_fail(self != NULL, NULL);
  sys_return_val_if_fail(self->data_type == SYS_VALUE_STRING, NULL);

  return self->v.v_string;
}

void sys_value_free(SysValue* self) {
  sys_return_if_fail(self != NULL);

  switch (self->data_type) {
    case SYS_VALUE_STRING:
      sys_clear_pointer(&self->v.v_string, sys_free);
      break;
    case SYS_VALUE_INT:
    case SYS_VALUE_DOUBLE:
    case SYS_VALUE_NULL:
    case SYS_VALUE_POINTER:
      break;
    default:
      break;
  }

  sys_free_N(self);
}

void sys_value_ref(SysValue* value) {
  sys_return_if_fail(value != NULL);
  sys_return_if_fail(SYS_REF_VALID_CHECK(value, MAX_REF_NODE));

  sys_ref_count_inc(value);
}

void sys_value_unref(SysValue* self) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(SYS_REF_VALID_CHECK(self, MAX_REF_NODE));

  if (!sys_ref_count_dec(self)) {
    return;
  }

  sys_value_free(self);
}
