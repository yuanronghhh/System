#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define SYS_OBJECT(o) ((SysObject *)o)
#define SYS_OBJECT_CLASS(o) ((SysObjectClass *)o)
#define SysType size_t
#define sys_type_from_instance(o) (((SysTypeInstance *)(o))->type_class->type)
#define sys_type_from_class(o) (((SysTypeClass *)(o))->type)
#define sys_instance_get_class(o, stype) ((stype *)((SysTypeInstance *)o)->type_class)

#define SYS_DEFINE_TYPE(ptype, TypeName, type_name)                 \
static void type_name##_init(SysObject *self);                      \
static void type_name##_class_init(TypeName##Class *self);          \
static void type_name##_finalize(SysObject *self);                  \
static SysTypeClass* type_name##_parent_class = NULL;               \
TypeName##Private * type_name##_get_private(TypeName* o) {          \
   return (TypeName##Private *)sys_type_get_private(((SysTypeInstance *)o), type_name##_get_type()); \
}                                                        \
static void type_name##_class_intern_init (SysPointer klass) \
{ \
  type_name##_parent_class = sys_type_class_ref(ptype); \
  type_name##_class_init ((TypeName##Class*) klass); \
} \
SysType type_name##_get_type(void) {                     \
  static SysType type = 0;                               \
  if(type != 0) {                                        \
    return type;                                         \
  }                                                      \
  const SysTypeInfo info = {                             \
      sizeof(TypeName##Class),                           \
      sizeof(TypeName),                                  \
      sizeof(TypeName##Private),                         \
      #TypeName,                                         \
      NULL,                                              \
      NULL,                                              \
      (SysTypeInitFunc)type_name##_class_intern_init,    \
      NULL,                                              \
      (SysInstanceInitFunc)type_name##_init              \
  };                                                     \
  type = sys_type_new((ptype), &info);                   \
  return type;                                           \
}

typedef struct _SysObject SysObject;
typedef struct _SysObjectProp SysObjectProp;

typedef struct _SysObjectClass SysObjectClass;
typedef SysPointer SysObjectPrivate;

typedef struct _SysTypeInfo SysTypeInfo;
typedef struct _SysTypeNode SysTypeNode;
typedef struct _SysTypeData SysTypeData;
typedef struct _SysTypeInstance SysTypeInstance;

typedef struct _SysTypeClass SysTypeClass;
typedef SysType (*SysTypeFunc)(void);
typedef void (*SysTypeInitFunc)(void *self);
typedef void (*SysTypeFinalizeFunc)(void *self);

typedef void (*SysInstanceInitFunc)(SysTypeInstance *self, void *cls);

struct _SysTypeInfo {
  int class_size;
  int instance_size;
  int private_size;
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

struct _SysObjectClass {
  SysTypeClass parent;

  void (*dispose)(SysObject *self);
};

struct _SysObject {
  SysTypeInstance instance;
  SysRef ref_count;
};

#define sys_object_unref(o) _sys_object_unref((SysObject*)o)
#define sys_object_ref(o) _sys_object_ref((SysObject*)o)

SYS_API SysType sys_object_get_type(void);
SYS_API void _sys_object_ref(SysObject* self);
SYS_API void _sys_object_unref(SysObject* self);
SYS_API void* sys_object_new(SysType type);

SYS_API void sys_type_init(void);
SYS_API void sys_type_deinit(void);

SYS_API SysType sys_type_new(SysType pnode, const SysTypeInfo *info);
SYS_API SysChar *sys_type_name(SysType type);
SYS_API SysType sys_type_get_by_name(const SysChar *name);
SYS_API SysTypeClass *sys_type_pclass(SysType type);
SYS_API void sys_type_node_unref(SysTypeNode *node);
SYS_API void sys_type_node_free(SysTypeNode *node);
SYS_API SysTypeInstance *sys_type_new_instance(SysType type);
SYS_API void sys_type_free_instance(SysTypeInstance *instance);
SYS_API void *sys_type_get_private(SysTypeInstance *instance, SysType type);
SYS_API void sys_type_class_unref(SysTypeClass *cls);
SYS_API void sys_type_class_free(SysTypeClass *cls);
SYS_API SysTypeClass *sys_type_class_ref(SysType type);

SYS_END_DECLS

#endif
