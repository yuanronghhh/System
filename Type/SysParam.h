#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

#include <System/Type/SysObject.h>

SYS_BEGIN_DECLS

#define SYS_TYPE_PARAM (sys_param_get_type())
#define SYS_PARAM(o) ((SysParam* )sys_object_cast_check(o, SYS_TYPE_PARAM))
#define SYS_PARAM_CLASS(o) ((SysParamClass *)sys_class_cast_check(o, SYS_TYPE_PARAM)
#define SYS_PARAM_GET_CLASS(o) sys_instance_get_class(o, SysParamClass)

struct _SysParamClass {
  SysObjectClass parent;
};

struct _SysParam {
  SysObject parent;

  /* < private > */
  SysType object_type;
  const SysChar *field_type_name;
  SysType field_type;
  const SysChar *field_name;
  SysInt offset;
};

struct _SysParamContext {
  SysType object_type;
  const SysChar *field_type_name;
  SysType field_type;
  const SysChar *field_name;
  SysInt offset;
};

SYS_API SysType sys_param_get_type(void);
SYS_API SysParam *sys_param_new(void);

SYS_API SysParam *sys_param_new_I(SysParamContext *info);

SYS_API const SysChar *sys_param_get_field_name(SysParam *self);
SYS_API SysInt sys_param_get_offset(SysParam *self);
SYS_API SysBool sys_param_set_value(SysParam *self, SysObject *o, SysValue *value);
SYS_API SysBool sys_param_get_value(SysParam *self, SysObject *o, SysValue **value);

SYS_END_DECLS

#endif
