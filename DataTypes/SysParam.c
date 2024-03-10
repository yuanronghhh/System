#include <System/DataTypes/SysParam.h>

#include <System/Utils/SysError.h>
#include <System/DataTypes/SysValue.h>

SYS_DEFINE_TYPE(SysParam, sys_param, SYS_TYPE_OBJECT);


const SysChar *sys_param_get_name(SysParam *self) {
  return self->field_name;
}

SysInt sys_param_get_offset(SysParam *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->offset;
}

SysBool sys_param_set_value(SysParam *self, SysObject *o, SysValue *value) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(o != NULL);
  sys_return_if_fail(value != NULL);

  SysInt vtype = sys_value_get_data_type(value);
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
    const SysChar *full_type,
    SysInt field_type,
    const SysChar *field_name,
    SysInt offset) {

  self->full_type = full_type;
  self->field_type = field_type;
  self->field_name = field_name;
  self->offset = offset;
}

SysParam* sys_param_new(void) {
  return sys_object_new(SYS_TYPE_PARAM, NULL);
}

SysParam *sys_param_new_I(
    const SysChar *full_type,
    SysInt field_type,
    const SysChar *field_name,
    SysInt offset) {
  SysParam *o = sys_param_new();

  sys_param_construct(o,
      full_type,
      field_type,
      field_name,
      offset);

  return o;
}

static void sys_param_dispose(SysObject* o) {
  SysParam *self = SYS_PARAM(o);

  SYS_OBJECT_CLASS(sys_param_parent_class)->dispose(o);
}

static void sys_param_class_init(SysParamClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_param_dispose;
}

void sys_param_init(SysParam* self) {
}

