#include <System/Type/SysParam.h>

#include <System/Utils/SysError.h>
#include <System/DataTypes/SysValue.h>

SYS_DEFINE_TYPE(SysParam, sys_param, SYS_TYPE_OBJECT);


const SysChar *sys_param_get_field_name(SysParam *self) {
  return self->field_name;
}

SysInt sys_param_get_offset(SysParam *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->offset;
}

SysBool sys_param_get_value(SysParam *self, SysObject *o, SysValue** value) {
  sys_return_val_if_fail(self != NULL, false);
  sys_return_val_if_fail(o != NULL, false);

  SysValue *v;
  SysPointer p;

  SysType tp = sys_type_from_instance(o);
  if(tp != self->type) {
    sys_warning_N("param is not object property: %s,%s",
        self->field_name, 
        sys_object_get_type_name(o));
    return false;
  }

  p = ((SysUInt8 *)o) + self->offset;
  v = sys_value_cast_value(p, self->field_type);
  if (v == NULL) { return false; }

  *value = v;
  return true;
}

SysBool sys_param_set_value(SysParam *self, SysObject *o, SysValue *value) {
  sys_return_val_if_fail(self != NULL, false);
  sys_return_val_if_fail(o != NULL, false);
  sys_return_val_if_fail(value != NULL, false);

  SysType vtype = sys_value_get_data_type(value);
  if(self->field_type != sys_value_get_data_type(value)) {
    sys_warning_N("failed to set property \"%s\": %s,%s",
        self->field_name,
        sys_value_get_type_name(self->field_type),
        sys_value_get_type_name(vtype));
    return false;
  }

  SysPointer p = ((SysUInt8 *)o) + self->offset;
  return sys_value_set_value(value, p);
}

/* object api */
static void sys_param_construct(SysParam *self,
    SysType type,
    const SysChar *full_type,
    SysType field_type,
    const SysChar *field_name,
    SysInt offset) {

  self->type = type;
  self->full_type = full_type;
  self->field_name = field_name;
  self->field_type = field_type;
  self->offset = offset;
}

SysParam* sys_param_new(void) {
  return sys_object_new(SYS_TYPE_PARAM, NULL);
}

SysParam *sys_param_new_I(
    SysType type,
    const SysChar *full_type,
    SysType field_type,
    const SysChar *field_name,
    SysInt offset) {
  SysParam *o = sys_param_new();

  sys_param_construct(o,
      type,
      full_type,
      field_type,
      field_name,
      offset);

  return o;
}

static void sys_param_dispose(SysObject* o) {

  SYS_OBJECT_CLASS(sys_param_parent_class)->dispose(o);
}

static void sys_param_class_init(SysParamClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_param_dispose;
}

void sys_param_init(SysParam* self) {
}

