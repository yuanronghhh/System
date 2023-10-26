#ifndef __SYS_JVALUE_H__
#define __SYS_JVALUE_H__

#include <System/Fundamental/SysCommonCore.h>
#include <System/DataTypes/SysArray.h>

SYS_BEGIN_DECLS

typedef struct _SysValue SysValue;

typedef enum _SYS_VALUE_ENUM {
  SYS_VALUE_NULL = 1,
  SYS_VALUE_BOOL,
  SYS_VALUE_STRING,
  SYS_VALUE_INT,
  SYS_VALUE_DOUBLE,
  SYS_VALUE_POINTER,
} SYS_VALUE_ENUM;

SysValue * sys_value_new(void);
SysValue * sys_value_copy(SysValue *self);
void sys_value_ref(SysValue* self);
void sys_value_unref(SysValue* self);
void sys_value_free(SysValue* self);

void sys_value_set_v_string(SysValue *self, const SysChar * v_string);
const SysChar * sys_value_get_v_string(SysValue *self);

void sys_value_set_v_pointer(SysValue *self, SysPointer v_pointer);
SysPointer sys_value_get_v_pointer(SysValue *self);

void sys_value_set_v_double(SysValue *self, SysDouble v_double);
SysDouble sys_value_get_v_double(SysValue *self);

void sys_value_set_v_int(SysValue *self, SysInt v_int);
SysInt sys_value_get_v_int(SysValue *self);

void sys_value_set_v_null(SysValue *self);
SysInt sys_value_get_data_type(SysValue *self);

SYS_END_DECLS

#endif
