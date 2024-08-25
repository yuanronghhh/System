#include <System/Type/SysObject.h>
#include <System/Type/SysParam.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/DataTypes/SysHArray.h>
#include <System/DataTypes/SysSList.h>
#include <System/Utils/SysString.h>
#include <System/Platform/Common/SysThread.h>

static void sys_object_base_class_init(SysObjectClass* self);
static void sys_object_base_finalize(SysObject* self);

static void sys_object_init(SysObject* self);
static void sys_object_class_init(SysObjectClass* self);

static SysTypeClass* sys_object_parent_class = NULL;
static SysInt SysObject_private_offset;
SysPointer sys_object_get_private(SysObject* o) {
  return (((SysUInt8*)o) + SysObject_private_offset);
}
static void sys_object_class_intern_init(SysPointer cls) {
  sys_object_parent_class = NULL;
  sys_type_class_adjust_private_offset(cls, &SysObject_private_offset);
  sys_object_class_init(cls);
}
void _sys_object_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_BASE_CLASS,
    sizeof(SysObjectClass),
    sizeof(SysObject),
    "SysObject",

    (SysTypeInitFunc)sys_object_base_class_init,
    (SysDestroyFunc)sys_object_base_finalize,

    (SysTypeInitFunc)sys_object_class_intern_init,
    NULL,
    (SysInstanceInitFunc)sys_object_init
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_OBJECT, &type_info);
}

static void sys_object_base_finalize(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_dispose_i(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

SysObject* _sys_object_new_from_instance(SysObject *o) {
  sys_return_val_if_fail(o != NULL, NULL);
  sys_return_val_if_fail(sgc_block_ref_check(o, MAX_REF_NODE), NULL);

  SysType type = sys_type_from_instance(o);
  return sys_object_new(type, NULL);
}

static SysObject* sys_object_dclone_i(SysObject *self) {
  sys_return_val_if_fail(self != NULL, NULL);

  return _sys_object_new_from_instance(self);
}

static void sys_object_finalize_i(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_base_class_init(SysObjectClass *self) {
  sys_return_if_fail(self != NULL);
}

SysObject* _sys_object_dclone(SysObject *o) {
  sys_return_val_if_fail(o != NULL, NULL);

  SysObjectClass *cls = SYS_OBJECT_GET_CLASS(o);
  sys_return_val_if_fail(cls->dclone != NULL, NULL);

  return cls->dclone(o);
}

static void object_destroy(SysObject* self) {
  sys_return_if_fail(self != NULL);
  SysObjectClass* cls;

  cls = SYS_OBJECT_GET_CLASS(self);
  while(cls) {

    if(cls->dispose) {

      cls->dispose(self);
    }

    cls = sys_class_get_parent_class(cls, SysObjectClass);
  }
}

void _sys_object_destroy(SysObject* self) {
  if(!sgc_block_destroy_check(self)) {
    return;
  }

  object_destroy(self);
}

static void sys_object_class_init(SysObjectClass *ocls) {
  ocls->dclone = sys_object_dclone_i;
  ocls->dispose = sys_object_dispose_i;
  ocls->finalize = sys_object_finalize_i;
}

static void sys_object_init(SysObject *self) {
}

SysType sys_object_get_type(void) {
  return SYS_TYPE_OBJECT;
}

void _sys_object_unref(SysObject* self) {
  sys_return_if_fail(self != NULL);

  if(!sgc_block_destroy_check(self)) {
    return;
  }

  object_destroy(self);
  sgc_block_free(self);
}

void * _sys_object_cast_check(SysObject* self, SysType ttype) {
  if (self == NULL) { return NULL; }
  sys_return_val_if_fail(sgc_block_ref_check(self, MAX_REF_NODE), NULL);

  SysType type = sys_type_from_instance(self);
  sys_assert(type != 0);

  SysTypeNode *node = sys_type_node(type);
  SysTypeNode* tnode;

  if(!sys_type_node_check(node)) {
    sys_error_N("%s", "Object check node Failed");
    return NULL;
  }

  if (!sgc_block_ref_cmp(self, 0)) {
    tnode = sys_type_node(ttype);

    if (sys_type_node_is(tnode, SYS_NODE_INTERFACE)) {

    } else {

      if (!sys_type_is_a(type, ttype)) {
        sys_error_N("Object check node failed: %p, %s to %s",
            self,
            sys_type_node_name(node),
            sys_type_node_name(tnode));
        return NULL;
      }
    }
  }

  return self;
}

void* _sys_class_cast_check(SysObjectClass* cls, SysType ttype) {
  sys_return_val_if_fail(cls != NULL, NULL);
  sys_return_val_if_fail(cls->dispose != NULL, NULL);

  SysType type = sys_type_from_class(cls);
  SysTypeNode* node = sys_type_node(type);
  SysTypeNode* tnode;

  if(!sys_type_node_check(node)
      || !sys_type_is_a(type, ttype)
    ) {
    tnode = sys_type_node(ttype);

    sys_error_N("Class check node Failed: %s to %s",
        sys_type_node_name(node),
        sys_type_node_name(tnode));
    return NULL;
  }

  return cls;
}

SysBool _sys_object_is_a(SysObject *self, SysType type) {
  sys_return_val_if_fail(sgc_block_ref_check(self, MAX_REF_NODE), false);

  SysType otype = sys_type_from_instance(self);

  return sys_type_is_a(otype, type);
}

const SysChar* _sys_object_get_type_name(SysObject *self) {
  sys_return_val_if_fail(self != NULL, NULL);
  sys_return_val_if_fail(sgc_block_ref_check(self, MAX_REF_NODE), NULL);

  SysType tp = sys_type_from_instance(self);
  SysTypeNode *node = sys_type_node(tp);

  return sys_type_node_name(node);
}

/* props */
void _sys_object_add_property(
    SysType type,
    const SysChar *full_type,
    SysInt field_type,
    const SysChar *field_name,
    SysInt offset) {

  sys_return_if_fail(type != 0);
  sys_return_if_fail(full_type != NULL);
  sys_return_if_fail(field_name != NULL);
  sys_return_if_fail(offset >= 0);

  SysParam *param = sys_param_new_I(type,
      full_type, 
      field_type, 
      field_name, 
      offset);

  if(param == NULL) {
    return;
  }

  sys_type_prop_set(type, param);
}

SysParam *sys_object_get_property(SysType type, const SysChar *name) {
  sys_return_val_if_fail(name != NULL, NULL);
  sys_return_val_if_fail(type != 0, NULL);

  return sys_type_prop_get(type, name);
}

SysHArray *sys_object_get_properties(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  return sys_type_get_properties(type);
}
