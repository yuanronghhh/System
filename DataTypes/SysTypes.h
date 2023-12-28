#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

/**
 * simple implement gobject for myself
 * and part of code copy from gobject
 *
 * see: ftp://ftp.gtk.org/pub/glib/
 */

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#define SYS_TYPE_FUNDAMENTAL_SHIFT 2
#define	SYS_TYPE_FUNDAMENTAL_MAX		(255 << SYS_TYPE_FUNDAMENTAL_SHIFT)
#define	TYPE_ID_MASK				((SysType) ((1 << SYS_TYPE_FUNDAMENTAL_SHIFT) - 1))

#define SYS_TYPE_INTERFACE ((SysType)(2 << SYS_TYPE_FUNDAMENTAL_SHIFT))
#define SYS_TYPE_OBJECT ((SysType)(20 << SYS_TYPE_FUNDAMENTAL_SHIFT))


#define sys_type_from_instance(o) (((SysTypeInstance *)(o))->type_class->type)
#define sys_type_from_class(o) (((SysTypeClass *)(o))->type)
#define sys_instance_get_class(o, TypeName) ((TypeName *)((SysTypeInstance *)o)->type_class)

#define sys_object_cast_check(o, stype) _sys_object_cast_check((SysObject *)o, stype)
#define sys_class_cast_check(cls, stype) _sys_class_cast_check((SysObjectClass *)cls, stype)

#define SYS_OBJECT(o) ((SysObject *)sys_object_cast_check(o, SYS_TYPE_OBJECT))
#define SYS_OBJECT_CLASS(cls) ((SysObjectClass *)sys_class_cast_check(cls, SYS_TYPE_OBJECT))
#define SYS_OBJECT_GET_CLASS(o) sys_instance_get_class(o, SysObjectClass)
#define SYS_TYPE_GET_INTERFACE(o, iface_type) _sys_type_get_interface((((SysTypeInstance *)o)->type_class), iface_type)

#define SYS_ADD_PRIVATE(TypeName) \
{ \
  TypeName##_private_offset = sizeof(TypeName##Private); \
}

#define SYS_DEFINE_BEGIN(TypeName, type_name, T_P, flag)  \
static void type_name##_init(TypeName *self);                          \
static void type_name##_class_init(TypeName##Class *o);                \
static void type_name##_dispose(SysObject *o);                         \
static SysTypeClass* type_name##_parent_class = NULL;                  \
static SysInt TypeName##_private_offset = 0;                           \
SysPointer type_name##_get_private(TypeName* o) {              \
   return (((SysUInt8 *)o) + TypeName##_private_offset); \
} \
static void type_name##_class_intern_init (SysPointer kclass) \
{ \
  type_name##_parent_class = sys_type_class_ref(T_P); \
  sys_type_class_adjust_private_offset (kclass, &TypeName##_private_offset); \
  type_name##_class_init ((TypeName##Class*) kclass); \
} \
SysType type_name##_get_type(void) {                     \
  static SysType type = 0;                               \
  if(type != 0) {                                        \
    return type;                                         \
  }                                                      \
  const SysTypeInfo type_info = {                        \
      sizeof(TypeName##Class),                           \
      sizeof(TypeName),                                  \
      #TypeName,                                         \
      NULL,                                              \
      NULL,                                              \
      (SysTypeInitFunc)type_name##_class_intern_init,    \
      NULL,                                              \
      (SysInstanceInitFunc)type_name##_init              \
  }; \
  type = sys_type_new((T_P), &type_info);

#define SYS_DEFINE_END() \
    return type; \
}

#define SYS_DEFINE_INTERFACE_BEGIN(TypeName, type_name, T_P) \
static void type_name##_default_init (TypeName##Interface *klass); \
SysType type_name##_get_type (void)                                \
{                                                                  \
  static SysType type = 0;                                         \
  if(type != 0) {                                                  \
    return type;                                                   \
  }                                                                \
  const SysTypeInfo type_info = {                                  \
      sizeof(TypeName##Interface),                                 \
      0,                                                           \
      #TypeName,                                                   \
      NULL,                                                        \
      NULL,                                                        \
      (SysTypeInitFunc)type_name##_default_init,                   \
      NULL,                                                        \
      NULL                                                         \
  };                                                               \
  type = sys_type_new(SYS_TYPE_INTERFACE, &type_info);             \

#define SYS_DEFINE_INTERFACE_END() \
  return type;                     \
}

#define SYS_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init)       { \
  const SysInterfaceInfo iface_info = {                         \
    (SysTypeInitFunc)(void (*)(void)) iface_init, NULL, NULL    \
  };                                                            \
  sys_type_imp_interface (type, TYPE_IFACE, &iface_info);       \
}

#define SYS_DEFINE_WITH_CODE(TypeName, type_name, T_P, _CODE_) SYS_DEFINE_BEGIN(TypeName, type_name, T_P, 0){_CODE_;}SYS_DEFINE_END()

#define SYS_DEFINE_TYPE_WITH_PRIVATE(TypeName, type_name, T_P) SYS_DEFINE_WITH_CODE(TypeName, type_name, T_P, SYS_ADD_PRIVATE(TypeName))

#define SYS_DEFINE_TYPE(TypeName, type_name, T_P) SYS_DEFINE_WITH_CODE(TypeName, type_name, T_P, {})

#define SYS_DEFINE_INTERFACE_WITH_CODE(TypeName, type_name, T_P, _CODE_) SYS_DEFINE_INTERFACE_BEGIN(TypeName, type_name, T_P){_CODE_;}SYS_DEFINE_INTERFACE_END()

#define SYS_DEFINE_INTERFACE(TypeName, type_name, T_P) SYS_DEFINE_INTERFACE_WITH_CODE(TypeName, type_name, T_P, ;)

typedef size_t SysType;
typedef struct _SysObject SysObject;

typedef struct _SysObjectClass SysObjectClass;

typedef struct _SysInterfaceInfo SysInterfaceInfo;
typedef struct _SysTypeInfo SysTypeInfo;
typedef struct _TypeNode TypeNode;
typedef union _SysTypeData SysTypeData;
typedef struct _SysTypeInstance SysTypeInstance;
typedef struct _SysTypeInterface SysTypeInterface;

typedef struct _SysTypeClass SysTypeClass;
typedef SysType (*SysTypeFunc) (void);
typedef void (*SysTypeInitFunc) (void *self);
typedef void (*SysTypeFinalizeFunc) (void *self);
typedef void (*SysInstanceInitFunc) (SysTypeInstance* self);
typedef void (*SysInterfaceInitFunc) (SysTypeInterface *self);
typedef void (*SysObjectFunc) (SysObject *o, ...);
typedef SysObject* (*SysCloneFunc) (SysObject *o);
typedef void (*SysRefHook) (SysObject *o, const SysChar *name, SysInt ref_count);

struct _SysInterfaceInfo {
  SysInterfaceInitFunc     interface_init;
  SysTypeFinalizeFunc      interface_finalize;
  SysPointer               interface_data;
};

struct _SysTypeInfo {
  SysInt class_size;
  SysInt instance_size;
  SysChar *name;

  SysTypeInitFunc class_base_init;
  SysTypeFinalizeFunc class_base_finalize;

  SysTypeInitFunc class_init;
  SysTypeFinalizeFunc class_finalize;

  SysInstanceInitFunc instance_init;
};

struct _SysTypeClass {
  SysType type;
};

struct _SysTypeInstance {
  SysTypeClass *type_class;
};

struct _SysTypeInterface {
  SysType type;         /* iface type */
  SysType instance_type;
  SysPointer vtable;
};

struct _SysObjectClass {
  SysTypeClass parent;

  SysObject *(*dclone) (SysObject *self);
  void (*dispose)(SysObject *self);
  void (*finalize)(SysObject *self);
};

struct _SysObject {
  SysTypeInstance instance;
  SysRef ref_count;
};

#define sys_object_unref(o) _sys_object_unref(SYS_OBJECT(o))
#define sys_object_ref(o) _sys_object_ref(SYS_OBJECT(o))
#define sys_object_is_a(o, type) _sys_object_is_a(SYS_OBJECT(o), type)
#define sys_object_dclone(o) _sys_object_dclone(SYS_OBJECT(o))
#define sys_object_get_type_name(o) _sys_object_get_type_name(SYS_OBJECT(o))

SYS_API void* sys_object_new(SysType type, const SysChar * first, ...);
SYS_API SysType sys_object_get_type(void);
SYS_API SysPointer _sys_object_ref(SysObject* self);
SYS_API void _sys_object_unref(SysObject* self);

SYS_API void sys_object_set_unref_hook(SysRefHook hook);
SYS_API void sys_object_set_ref_hook(SysRefHook hook);
SYS_API void sys_object_set_new_hook(SysRefHook hook);

SYS_API void * _sys_object_cast_check(SysObject* self, SysType ttype);
SYS_API void * _sys_class_cast_check(SysObjectClass* cls, SysType ttype);
SYS_API SysBool _sys_object_is_a(SysObject *self, SysType type);

SYS_API void sys_type_setup(void);
SYS_API void sys_type_teardown(void);

SYS_API SysType sys_type_new(SysType pnode, const SysTypeInfo *info);
SYS_API SysObject* _sys_object_dclone(SysObject *o);

SYS_API const SysChar* _sys_object_get_type_name(SysObject* self);
SYS_API SysTypeInterface* _sys_type_get_interface(SysTypeClass *cls, SysType iface_type);
SYS_API void sys_type_imp_interface(SysType instance_type, SysType iface_type, const SysInterfaceInfo* info);

SYS_API SysChar *sys_type_name(SysType type);
SYS_API SysType sys_type_get_by_name(const SysChar *name);
SYS_API SysTypeClass *sys_type_pclass(SysType type);
SYS_API void sys_type_node_unref(TypeNode *node);
SYS_API void sys_type_node_free(TypeNode *node);
SYS_API SysTypeInstance *sys_type_new_instance(SysType type);
SYS_API void sys_type_free_instance(SysTypeInstance *instance);
SYS_API void *sys_type_get_private(SysTypeInstance *instance, SysType type);
SYS_API void sys_type_class_unref(SysTypeClass *cls);
SYS_API void sys_type_class_free(SysTypeClass *cls);
SYS_API void sys_type_class_adjust_private_offset (SysTypeClass *cls, SysInt * private_offset);
SYS_API SysTypeClass *sys_type_class_ref(SysType type);
SYS_API SysBool sys_type_is_a(SysType child, SysType parent);

SYS_END_DECLS

#endif
