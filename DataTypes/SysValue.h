#ifndef __SYS_JVALUE_H__
#define __SYS_JVALUE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define SysJBool bool
#define SysJDouble SysDouble
#define SysJInt SysInt
#define SysJPointer SysPointer
typedef struct _SysJSource SysJSource;
typedef struct _SysValue SysValue;
typedef struct _SysJPair SysJPair;
typedef SysPtrArray SysJArray;
typedef SysPtrArray SysJObject;
typedef SysChar SysJChar;

typedef enum _SYS_JVALUE_ENUM {
  SYS_BOOL = 1,
  SYS_STRING,
  SYS_POINTER,
  SYS_NULL,
  SYS_JARRAY,
  SYS_JOBJECT,
  SYS_JSOURCE,

  SYS_JINT,
  SYS_JDOUBLE,
  SYS_JPROPERTY,
  SYS_JPAIR,
  SYS_JNODE,

  SYS_JINVALID
} SYS_VALUE_ENUM;

SysInt sys_value_data_type(SysValue * o);
const SysChar * sys_value_v_string(SysValue * o);
SysValue * sys_value_new(void);
SysValue * sys_value_copy(SysValue * o);
void sys_value_ref(SysValue* value);
void sys_value_unref(SysValue* value);
SysValue * sys_value_new_string(const SysChar * s);
SysValue * sys_value_new_pointer(const SysPointer ptr);
void sys_value_free(SysValue* value);

SYS_END_DECLS

#endif
