#include <System/DataTypes/SysValue.h>
#include <System/Utils/SysString.h>

struct _SysJSource {
  SysChar *string;
  SysInt startline;
};

struct _SysJPair {
  SysChar *key;
  SysValue *prop;
  SysValue *value;
};

struct _SysJObject {
  SysJPair **pairs;
  SysInt len;
};

struct _SysValue {
  SYS_VALUE_ENUM type;

  union {
    SysValue *v_node;
    SysJSource *v_source;
    SysJPointer v_pointer;
    SysJBool v_bool;
    SysJDouble v_double;
    SysJInt v_int;
    SysJChar* v_string;
    SysJPair *v_pair;
    SysJObject *v_object;
    SysJArray *v_array;
  } v;

  SysRef ref_count;
};

SysInt sys_value_data_type(SysValue *o) {
  sys_return_val_if_fail(o != NULL, -1);

  return o->type;
}

const SysChar* sys_value_v_string(SysValue *o) {
  return o->v.v_string;
}

SysValue* sys_value_new(void) {
  SysValue *o = sys_new0_N(SysValue, 1);

  sys_ref_count_init(o);

  return o;
}

SysValue *sys_value_copy(SysValue *o) {
  sys_return_val_if_fail(o != NULL, NULL);
  
  SysValue *n = NULL;

  switch (o->type) {
    case SYS_STRING:
      n = sys_value_new_string(o->v.v_string);
      break;
    default:
      break;
  }

  return n;
}

SysValue* sys_value_new_string(const SysChar *s) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysValue *o = sys_value_new();

  o->type = SYS_STRING;
  o->v.v_string = sys_strdup(s);

  return o;
}

void sys_value_free(SysValue* value) {
  sys_return_if_fail(value);

  switch(value->type) {
    case SYS_STRING:
      sys_free_N(value->v.v_string);
      break;
    default:
      break;
  }

  sys_free_N(value);
}

void sys_value_ref(SysValue* value) {
  sys_return_if_fail(value != NULL);
  sys_return_if_fail(SYS_REF_VALID_CHECK(value, MAX_REF_NODE));

  sys_ref_count_inc(value);
}

void sys_value_unref(SysValue* value) {
  sys_return_if_fail(value != NULL);
  sys_return_if_fail(SYS_REF_VALID_CHECK(value, MAX_REF_NODE));

  if (!sys_ref_count_dec(value)) {
    return;
  }

  sys_value_free(value);
}
