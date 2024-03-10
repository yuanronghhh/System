#include <System/DataTypes/SysParam.h>

#include <System/Utils/SysError.h>


SYS_DEFINE_TYPE(SysParam, sys_param, SYS_TYPE_OBJECT);


const SysChar *sys_param_get_name(SysParam *self) {
  return self->field_name;
}

SysInt sys_param_get_offset(SysParam *self) {
  sys_return_val_if_fail(self != NULL, -1);

  return self->offset;
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

