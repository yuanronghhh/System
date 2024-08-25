#ifndef __SYS_OBJECT_H__
#define __SYS_OBJECT_H__

#include <System/Type/SysType.h>

SYS_BEGIN_DECLS

#define sys_object_cast_check(o, stype) _sys_object_cast_check((SysObject *)o, stype)
#define sys_class_cast_check(cls, stype) _sys_class_cast_check((SysObjectClass *)cls, stype)

#define SYS_OBJECT(o) ((SysObject *)sys_object_cast_check(o, SYS_TYPE_OBJECT))
#define SYS_OBJECT_CLASS(cls) ((SysObjectClass *)sys_class_cast_check(cls, SYS_TYPE_OBJECT))
#define SYS_OBJECT_GET_CLASS(o) sys_instance_get_class(o, SysObjectClass)

struct _SysObjectClass {
  SysTypeClass parent;

  SysObject* (*dclone) (SysObject* self);
  void (*dispose) (SysObject *self);
  void (*finalize) (SysObject *self);
};

struct _SysObject {
  SysTypeInstance instance;
};

#define sys_object_create(o, type) sgc_block_create(o, type, NULL)
#define sys_object_destroy(o) _sys_object_destroy(SYS_OBJECT(o))
#define sys_object_unref(o) _sys_object_unref(SYS_OBJECT(o))
#define sys_object_ref(o) _sgc_block_ref(SGC_BLOCK(o))
SYS_API void _sys_object_unref(SysObject* self);
SYS_API void _sys_object_destroy(SysObject* self);

#define sys_object_is_a(o, type) _sys_object_is_a(SYS_OBJECT(o), type)
#define sys_object_dclone(o) _sys_object_dclone(SYS_OBJECT(o))
#define sys_object_get_type_name(o) _sys_object_get_type_name(SYS_OBJECT(o))

SYS_API SysType sys_object_get_type(void);
#define sys_object_new(tp, ...) sgc_block_new(tp, __VA_ARGS__)

#define sys_object_set_unref_hook(hook) sgc_block_set_unref_hook((SysRefHook)hook)
#define sys_object_set_ref_hook(hook) sgc_block_set_ref_hook((SysRefHook)hook)
#define sys_object_set_new_hook(hook) sgc_block_set_new_hook((SysRefHook)hook)

SYS_API void * _sys_object_cast_check(SysObject* self, SysType ttype);
SYS_API void * _sys_class_cast_check(SysObjectClass* cls, SysType ttype);
SYS_API SysBool _sys_object_is_a(SysObject *self, SysType type);
SYS_API void _sys_object_init_type(void);

#define sys_object_add_property(TYPE, TypeName, full_type, field_type, field_name) \
  _sys_object_add_property(TYPE, full_type, field_type, #field_name, offsetof(TypeName, field_name))

SYS_API void _sys_object_add_property(
    SysType type,
    const SysChar *full_type,
    SysInt field_type,
    const SysChar *field_name,
    SysInt offset);
SYS_API SysParam *sys_object_get_property(SysType type, const SysChar *name);
SYS_API SysHArray *sys_object_get_properties(SysType type);

SYS_API SysObject* _sys_object_dclone(SysObject *o);
SYS_API const SysChar* _sys_object_get_type_name(SysObject* self);

SYS_END_DECLS

#endif
