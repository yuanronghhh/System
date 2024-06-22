#ifndef __SYS_TYPE_H__
#define __SYS_TYPE_H__

#include <System/Type/SysGc.h>

SYS_BEGIN_DECLS

#define SYS_TYPE_FUNDAMENTAL_SHIFT 2
#define SYS_TYPE_FUNDAMENTAL_MAX  (255 << SYS_TYPE_FUNDAMENTAL_SHIFT)
#define TYPE_ID_MASK    ((SysType) ((1 << SYS_TYPE_FUNDAMENTAL_SHIFT) - 1))

#define SYS_TYPE_MAKE_FUNDAMENTAL(x)              ((SysType) ((x) << SYS_TYPE_FUNDAMENTAL_SHIFT))
#define SYS_TYPE_FUNDAMENTAL_NODE                 SYS_TYPE_MAKE_FUNDAMENTAL(1)
#define SYS_TYPE_INTERFACE                        SYS_TYPE_MAKE_FUNDAMENTAL(2)
#define SYS_TYPE_CHAR                             SYS_TYPE_MAKE_FUNDAMENTAL(3)
#define SYS_TYPE_BOOL                             SYS_TYPE_MAKE_FUNDAMENTAL(5)
#define SYS_TYPE_INT                              SYS_TYPE_MAKE_FUNDAMENTAL(6)
#define SYS_TYPE_DOUBLE                           SYS_TYPE_MAKE_FUNDAMENTAL(15)
#define SYS_TYPE_STRING                           SYS_TYPE_MAKE_FUNDAMENTAL(16)
#define SYS_TYPE_POINTER                          SYS_TYPE_MAKE_FUNDAMENTAL(17)
#define SYS_TYPE_OBJECT                           SYS_TYPE_MAKE_FUNDAMENTAL(20)

#define sys_type_from_instance(o) sys_type_from_sgc_block(o)
#define sys_type_from_class(o) sys_type_from_sgc_block(o)
#define sys_instance_get_class(o, TypeName) ((TypeName *)((SysTypeInstance *)o)->type_class)

#define SYS_TYPE_GET_INTERFACE(o, iface_type) _sys_type_get_interface((((SysTypeInstance *)o)->parent.type), iface_type)

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
      SYS_NODE_CLASS,                                    \
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
      SYS_NODE_INTERFACE,                                          \
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
    (SysInterfaceInitFunc)iface_init, NULL, NULL                \
  };                                                            \
  sys_type_imp_interface (type, TYPE_IFACE, &iface_info);       \
}

#define SYS_DEFINE_WITH_CODE(TypeName, type_name, T_P, _CODE_) SYS_DEFINE_BEGIN(TypeName, type_name, T_P, 0){_CODE_;}SYS_DEFINE_END()

#define SYS_DEFINE_TYPE_WITH_PRIVATE(TypeName, type_name, T_P) SYS_DEFINE_WITH_CODE(TypeName, type_name, T_P, SYS_ADD_PRIVATE(TypeName))

#define SYS_DEFINE_TYPE(TypeName, type_name, T_P) SYS_DEFINE_WITH_CODE(TypeName, type_name, T_P, {})

#define SYS_DEFINE_INTERFACE_WITH_CODE(TypeName, type_name, T_P, _CODE_) SYS_DEFINE_INTERFACE_BEGIN(TypeName, type_name, T_P){_CODE_;}SYS_DEFINE_INTERFACE_END()

#define SYS_DEFINE_INTERFACE(TypeName, type_name, T_P) SYS_DEFINE_INTERFACE_WITH_CODE(TypeName, type_name, T_P, ;)


struct _SysInterfaceInfo {
  SysInterfaceInitFunc     interface_init;
  SysTypeFinalizeFunc      interface_finalize;
  SysPointer               interface_data;
};

struct _SysTypeInfo {
  SysInt node_type;
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
  SgcBlock parent;

  /* limit 255 interface count for class */
  SysUInt8 n_ifaces;
  IFaceEntry **ifaces;
};

struct _SysTypeInstance {
  SgcBlock parent;
  SysTypeClass *type_class;
};

struct _SysTypeInterface {
  SysType type;         /* iface type */
};

SYS_API void sys_type_setup(void);
SYS_API void sys_type_teardown(void);

SYS_API SysType sys_type_new(SysType pnode, const SysTypeInfo *info);

SYS_API const SysChar* _sys_object_get_type_name(SysObject* self);
SYS_API SysTypeInterface* _sys_type_get_interface(SysType type, SysType iface_type);
SYS_API void sys_type_imp_interface(SysType instance_type, SysType iface_type, const SysInterfaceInfo* info);

SYS_API SysTypeNode* sys_type_node(SysType type);
SYS_API const SysChar *sys_type_node_name(SysTypeNode *node);
SYS_API SysBool sys_type_node_check(SysTypeNode *node);
SYS_API SysBool sys_type_node_is(SysTypeNode *node, SYS_NODE_ENUM tp);

SYS_API SysType sys_type_get_by_name(const SysChar *name);
SYS_API SysTypeClass *sys_type_pclass(SysType type);
SYS_API void sys_type_node_unref(SysTypeNode *node);
SYS_API void sys_type_node_free(SysTypeNode *node);

SYS_API SysBool sys_type_instance_create(SysTypeInstance *instance, SysType type);
SYS_API SysTypeInstance *sys_type_instance_new(SysType type, SysSize count);
SYS_API SysBool sys_type_instance_get_size(SysType type,
    SysSize *size, 
    SysSize *priv_size);
SYS_API void sys_type_instance_free(SysTypeInstance *instance);

SYS_API void *sys_type_get_private(SysTypeInstance *instance, SysType type);
SYS_API void sys_type_class_unref(SysTypeClass *cls);
SYS_API void sys_type_class_free(SysTypeClass *cls);
SYS_API void sys_type_class_adjust_private_offset (SysTypeClass *cls, SysInt * private_offset);
SYS_API SysTypeClass *sys_type_class_ref(SysType type);
SYS_API SysBool sys_type_is_a(SysType child, SysType parent);
SYS_API SysTypeNode * sys_type_make_fundamental_node(const SysTypeNode * pnode, SysType ftype, const SysTypeInfo * info);
SYS_API void sys_type_prop_set(SysType tp, SysParam *param);
SYS_API SysParam* sys_type_prop_get(SysType tp, const SysChar *name);
SYS_API SysHArray *sys_type_get_properties(SysType type);

SYS_END_DECLS

#endif
