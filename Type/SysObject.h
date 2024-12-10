#ifndef __SYS_OBJECT_H__
#define __SYS_OBJECT_H__

#include <System/Type/SysType.h>

SYS_BEGIN_DECLS

#define sys_object_cast_check(o, stype) _sys_object_cast_check((SysObject *)o, stype)
#define sys_class_cast_check(cls, stype) _sys_class_cast_check((SysObjectClass *)cls, stype)

#define SYS_OBJECT(o) ((SysObject *)sys_object_cast_check(o, SYS_TYPE_OBJECT))
#define SYS_IS_OBJECT(o) (sys_object_cast_check(o, SYS_TYPE_OBJECT))
#define SYS_OBJECT_CLASS(cls) ((SysObjectClass *)sys_class_cast_check(cls, SYS_TYPE_OBJECT))
#define SYS_OBJECT_GET_CLASS(o) sys_instance_get_class(o, SysObjectClass)

struct _SysObjectClass {
  SysTypeClass parent;

  SysPointer (*ref) (SysObject* self);
  void (*unref) (SysObject* self);

  void (*destroy) (SysObject* self);
  SysObject* (*dclone) (SysObject* self);
  void (*dispose) (SysObject *self);
  void (*finalize) (SysObject *self);
};

struct _SysObject {
  SysTypeInstance instance;
};

#define sys_object_create(o, type) _sys_object_create((SysObject *)o, type)
#define sys_object_destroy(o) _sys_object_destroy((SysObject *)o)
#define sys_object_unref(o) _sys_object_unref((SysObject *)o)
#define sys_object_ref(o) _sys_object_ref((SysObject *)o)
SYS_API void _sys_object_unref(SysObject* self);
SYS_API SysPointer _sys_object_ref(SysObject* self);

SYS_API void _sys_object_create(SysObject* self, SysType type);
SYS_API void _sys_object_destroy(SysObject* self);

#define sys_object_is_a(o, type) _sys_object_is_a((SysObject *)o, type)
#define sys_object_dclone(o) _sys_object_dclone((SysObject *)o)
#define sys_object_get_type_name(o) _sys_object_get_type_name((SysObject *)o)

SYS_API SysType sys_object_get_type(void);
SYS_API SysPointer sys_object_new(SysType type, const SysChar * first, ...);

void sys_object_set_unref_hook(SysObjectRefHook hook);
void sys_object_set_ref_hook(SysObjectRefHook hook);
void sys_object_set_new_hook(SysObjectRefHook hook);

SYS_API void * _sys_object_cast_check(SysObject* self, SysType ttype);
SYS_API void * _sys_class_cast_check(SysObjectClass* cls, SysType ttype);
SYS_API SysBool _sys_object_is_a(SysObject *self, SysType type);
SYS_API void _sys_object_init_type(void);

#define SYS_OBJECT_PROPERTY_BEGIN { \
  SysParamContext priv_param = {0}; \

#define SYS_OBJECT_PROPERTY_END }

#define sys_object_add_property(TYPE, TypeName, m_field_type_name, M_FIELD_TYPE, m_field)       \
  priv_param.object_type = TYPE;                                                                \
  priv_param.field_type_name = m_field_type_name;                                               \
  priv_param.field_type = M_FIELD_TYPE;                                                         \
  priv_param.field_name = #m_field;                                                             \
  priv_param.offset = offsetof(TypeName, m_field);                                              \
  _sys_object_add_property(&priv_param);

SYS_API void _sys_object_add_property(SysParamContext *info);
SYS_API SysParam *sys_object_get_property(SysType type, const SysChar *name);
SYS_API SysHArray *sys_object_get_properties(SysType type);

SYS_API SysObject* _sys_object_dclone(SysObject *o);
SYS_API const SysChar* _sys_object_get_type_name(SysObject* self);

SYS_END_DECLS

#endif
