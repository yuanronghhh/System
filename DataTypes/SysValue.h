#ifndef __SYS_JVALUE_H__
#define __SYS_JVALUE_H__

#include <System/Fundamental/SysCommonCore.h>
#include <System/DataTypes/SysArray.h>
#include <System/DataTypes/SysTypes.h>


SYS_BEGIN_DECLS

typedef enum _SYS_VALUE_ENUM {
  SYS_VALUE_NULL = 0,
  SYS_VALUE_BOOL = SYS_TYPE_BOOL,
  SYS_VALUE_STRING = SYS_TYPE_STRING,
  SYS_VALUE_INT = SYS_TYPE_INT,
  SYS_VALUE_DOUBLE = SYS_TYPE_DOUBLE,
  SYS_VALUE_POINTER = SYS_TYPE_POINTER,
  SYS_VALUE_OBJECT = SYS_TYPE_OBJECT,
} SYS_VALUE_ENUM;

SysValue * sys_value_new(void);
SysValue * sys_value_copy(SysValue *self);
void sys_value_ref(SysValue* self);
void sys_value_unref(SysValue* self);
void sys_value_free(SysValue* self);

SysValue *sys_value_new_int(SysInt v);
SysValue *sys_value_new_string(const SysChar *v);
SysValue *sys_value_new_pointer(const SysPointer v);
SysValue *sys_value_new_bool(SysBool v);
SysValue *sys_value_new_double(SysDouble v);
SysValue *sys_value_new_null(void);
SysValue *sys_value_new_object(SysObject* v);

SysValue* sys_value_cast_value(SysPointer *p, SysType data_type);
SysBool sys_value_set_value(SysValue *self, SysPointer *p);
const SysChar *sys_value_get_type_name(SysType data_type);

void sys_value_set_v_string(SysValue *self, const SysChar * v_string);
const SysChar * sys_value_get_v_string(SysValue *self);

void sys_value_set_v_pointer(SysValue *self, SysPointer v_pointer);
SysPointer sys_value_get_v_pointer(SysValue *self);

void sys_value_set_v_double(SysValue *self, SysDouble v_double);
SysDouble sys_value_get_v_double(SysValue *self);

void sys_value_set_v_int(SysValue *self, SysInt v_int);
SysInt sys_value_get_v_int(SysValue *self);

void sys_value_set_v_null(SysValue *self);
SysType sys_value_get_data_type(SysValue *self);

void sys_value_set_v_bool(SysValue *self, SysBool v_bool);
SysBool sys_value_get_v_bool(SysValue *self);

void sys_value_set_v_object(SysValue *self, SysObject * v_object);
SysObject * sys_value_get_v_object(SysValue *self);

SYS_END_DECLS

#endif
