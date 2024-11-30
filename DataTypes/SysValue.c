#include <System/DataTypes/SysValue.h>
#include <System/Type/SysObject.h>
#include <System/Utils/SysStr.h>

struct _SysValue {
  SysType data_type;

  union {
    SysPointer v_pointer;
    SysObject* v_object;
    SysBool v_bool;
    SysDouble v_double;
    SysInt v_int;
    SysChar* v_string;
  } v;

  SysRef ref_count;
};

SysType sys_value_get_data_type(SysValue *self) {
  sys_return_val_if_fail(self != NULL, 0);

  return self->data_type;
}

void sys_value_set_v_object(SysValue *self, SysObject * v_object) {
  sys_return_if_fail(self != NULL);

  self->data_type = SYS_VALUE_OBJECT;
  self->v.v_object = v_object;
}

SysObject * sys_value_get_v_object(SysValue *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  return self->v.v_object;
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

const SysChar *sys_value_get_type_name(SysType data_type) {
  switch (data_type) {
    case SYS_VALUE_STRING:
      return "string";
    case SYS_VALUE_INT:
      return "int";
    case SYS_VALUE_DOUBLE:
      return "double";
    case SYS_VALUE_NULL:
      return "null";
    case SYS_VALUE_POINTER:
      return "pointer";
    case SYS_VALUE_OBJECT:
      return "object";
    default:
      sys_warning_N("Not support system value type: %d", data_type);
      return NULL;
  }
}

SysValue* sys_value_cast_value(SysPointer *p, SysType data_type) {
  switch (data_type) {
    case SYS_VALUE_STRING:
      return sys_value_new_string(*((SysChar **)p));
    case SYS_VALUE_INT:
      return sys_value_new_int(*((SysInt *)p));
    case SYS_VALUE_DOUBLE:
      return sys_value_new_double(*((SysDouble *)p));
    case SYS_VALUE_NULL:
      return sys_value_new_null();
    case SYS_VALUE_POINTER:
      return sys_value_new_pointer(*p);
    case SYS_VALUE_OBJECT:
      return sys_value_new_object(*p);
    default:
      sys_warning_N("Not support system value type: %d", data_type);
      return NULL;
  }

  return NULL;
}

SysBool sys_value_set_value(SysValue *self, SysPointer *p) {
  switch (self->data_type) {
    case SYS_VALUE_STRING:
      *((SysChar **)p) = self->v.v_string;
      break;
    case SYS_VALUE_INT:
      *((SysInt *)p) = self->v.v_int;
      break;
    case SYS_VALUE_DOUBLE:
      *((SysDouble *)p) = self->v.v_double;
      break;
    case SYS_VALUE_NULL:
      *p = NULL;
      break;
    case SYS_VALUE_POINTER:
      *p = self->v.v_pointer;
      break;
    case SYS_VALUE_OBJECT:
      *((SysObject **)p) = self->v.v_object;
      break;
    default:
      sys_warning_N("Not support system value type: %d", self->data_type);
      return false;
  }

  return true;
}

SysValue* sys_value_new(void) {
  SysValue *o = sys_new0(SysValue, 1);

  sys_block_ref_init(o);

  return o;
}

SysValue *sys_value_new_int(SysInt v) {
  SysValue *o = sys_value_new();
  sys_value_set_v_int(o, v);
  return o;
}

SysValue *sys_value_new_string(const SysChar *v) {
  SysValue *o = sys_value_new();
  sys_value_set_v_string(o, v);
  return o;
}

SysValue *sys_value_new_pointer(const SysPointer v) {
  SysValue *o = sys_value_new();
  sys_value_set_v_pointer(o, v);
  return o;
}

SysValue *sys_value_new_object(SysObject* v) {
  SysValue *o = sys_value_new();
  sys_value_set_v_object(o, v);
  return o;
}

SysValue *sys_value_new_bool(SysBool v) {
  SysValue *o = sys_value_new();
  sys_value_set_v_bool(o, v);
  return o;
}

SysValue *sys_value_new_double(SysDouble v) {
  SysValue *o = sys_value_new();
  sys_value_set_v_double(o, v);
  return o;
}

SysValue *sys_value_new_null(void) {
  SysValue *o = sys_value_new();
  sys_value_set_v_null(o);
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
    case SYS_VALUE_OBJECT:
      sys_value_set_v_object(nself, oself->v.v_object);
      break;
    default:
      sys_warning_N("Not support system value type: %d", oself->data_type);
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

void sys_value_set_v_bool(SysValue *self, SysBool v_bool) {
  sys_return_if_fail(self != NULL);

  self->v.v_bool = v_bool;
}

SysBool sys_value_get_v_bool(SysValue *self) {
  sys_return_val_if_fail(self != NULL, false);

  return self->v.v_bool;
}

void sys_value_set_v_null(SysValue *self) {
  sys_return_if_fail(self != NULL);

  self->data_type = SYS_VALUE_NULL;
  self->v.v_pointer = NULL;
}

void sys_value_set_v_string(SysValue *self, const SysChar * v_string) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(v_string != NULL);

  sys_assert(self->v.v_string == NULL);

  self->data_type = SYS_VALUE_STRING;
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
    case SYS_VALUE_OBJECT:
      sys_clear_pointer(&self->v.v_object, _sys_object_unref);
      break;
    default:
      sys_warning_N("Not support system value type: %d", self->data_type);
      break;
  }

  sys_free(self);
}

void sys_value_ref(SysValue* value) {
  sys_return_if_fail(value != NULL);
  sys_return_if_fail(sys_block_valid_check(value));

  sys_block_ref_inc(value);
}

void sys_value_unref(SysValue* self) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(sys_block_valid_check(self));

  if (!sys_block_ref_dec(self)) {
    return;
  }

  sys_value_free(self);
}
