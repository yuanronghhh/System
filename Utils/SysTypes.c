#include <DataTypes/SysHashTable.h>
#include <Utils/SysString.h>
#include <Utils/SysTypes.h>

#define NODE_TYPE(o) ((SysType)(o))
#define NODE_PARENT(node) (node->supers[1])

typedef struct _SysInstanceData SysInstanceData;

struct _SysInstanceData {
  SysInt class_size;
  SysInt instance_size;
  SysInt private_size;
  SysInt prop_offset;
  SysTypeInitFunc class_init;  // set class_intern_init
  SysInstanceInitFunc instance_init;
  SysTypeFinalizeFunc class_finalize;
  void *class_ptr;
  SysObjectProp *props_ptr;
};

struct _SysTypeData {
  SysRef ref_count;

  SysInstanceData v;
};

struct _SysTypeNode {
  SysChar *name;

  SysTypeData data;

  SysRef ref_count;
  SysInt n_supers;
  SysType supers[1];
};

static SysHashTable *ht = NULL;

static void sys_object_base_class_init(SysObjectClass *self);
static void sys_object_base_finalize(SysObject *self);

static inline SysTypeNode* sys_type_node(SysType type);
static void sys_object_init(SysObject *self);
static void sys_object_dispose(SysObject *self);
static void sys_object_class_init(SysObjectClass *self);

static SysTypeClass* sys_object_parent_class = NULL;
SysObjectPrivate * sys_object_get_private(SysObject* o) {
   return (SysObjectPrivate *)sys_type_get_private(((SysTypeInstance *)o), sys_object_get_type()); 
}
static void sys_object_class_intern_init(SysPointer cls) {
  SysTypeNode *node = sys_type_node(sys_type_from_class(cls));
  SysTypeNode *pnode = sys_type_node(NODE_PARENT(node));

  sys_object_parent_class = NULL; // sys_type_class_ref(NODE_TYPE(pnode));
  sys_object_class_init(cls);
}
SysType sys_object_get_type(void) {
  static SysType type = 0;
  if (type != 0) {
    return type;
  }

  const SysTypeInfo info = {
      sizeof(SysObjectClass),
      sizeof(SysObject),
      sizeof(SysObjectPrivate),
      "SysObject",
      (SysTypeInitFunc)sys_object_base_class_init,
      (SysDestroyFunc)sys_object_base_finalize,
      (SysTypeInitFunc)sys_object_class_intern_init,
      NULL,
      (SysInstanceInitFunc)sys_object_init,
  };

  type = sys_type_new(0, &info);
  return type;
}

static void sys_object_free(SysObject *self) {
  sys_return_if_fail(self != NULL);

}

static void sys_object_base_finalize(SysObject *self) {
  sys_return_if_fail(self != NULL);

}

static void sys_object_real_dispose(SysObject *self) {
  sys_return_if_fail(self != NULL);
  SysObjectClass *ocls = sys_instance_get_class(self, SysObjectClass);
}

static void sys_object_dispose(SysObject *self) {
  sys_return_if_fail(self != NULL);
  SysObjectClass *ocls = sys_instance_get_class(self, SysObjectClass);

  if (ocls != NULL) {
    ocls->dispose(self);
  }
}

static void sys_object_base_class_init(SysObjectClass *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_class_init(SysObjectClass *self) {
  self->dispose = sys_object_real_dispose;
}

static void sys_object_init(SysObject *self) {
}

void* sys_object_new(SysType type) {
  SysObject *o = (SysObject *)sys_type_new_instance(type);
  if (o == NULL) { return NULL; }

  sys_ref_count_init(o);

  return o;
}

void _sys_object_ref(SysObject* self) {
  sys_ref_count_inc(self);
}

void _sys_object_unref(SysObject* self) {
  SysObjectClass *cls;

  cls = sys_instance_get_class(self, SysObjectClass);

  if (cls->dispose) {
    cls->dispose(self);
  }

  sys_type_free_instance((SysTypeInstance *)self);
}

/* SysType */
SysType sys_type_new(SysType ptype, const SysTypeInfo *info) {
  SysTypeNode *pnode;
  SysTypeNode *node;
  int nodesize = 0;
  int pn_supers = 0;
  int ppsize = 0;

  pnode = sys_type_node(ptype);
  if (pnode) {
    pn_supers = pnode->n_supers + 1;
    ppsize = pnode->data.v.private_size;
  }

  nodesize = sizeof(SysTypeNode) + sizeof(SysType) * (pn_supers + 1);
  node = sys_malloc0_N(nodesize);
  node->name = sys_strdup(info->name);

  node->data.v.instance_size = info->instance_size;
  node->data.v.private_size = ppsize + info->private_size;
  node->data.v.prop_offset = 0;
  node->data.v.class_size = info->class_size;
  node->data.v.class_init = info->class_init;
  node->data.v.class_finalize = info->class_finalize;
  node->data.v.instance_init = info->instance_init;
  node->n_supers = pn_supers;

  node->supers[0] = (SysType)node;
  if (pnode) {
    sys_memcpy(node->supers + 1, sizeof(SysType) * (node->n_supers), pnode->supers, sizeof(SysType) * node->n_supers);
  } else {
    node->supers[1] = 0;
  }

  sys_ref_count_init(&node->data);
  sys_ref_count_init(node);

  assert(ht != NULL && "sys types should init before use.");
  sys_hash_table_insert(ht, sys_strdup(node->name), node);

  return (SysType)node;
}

void sys_type_node_free(SysTypeNode *node) {
  assert(node->ref_count >= SYS_REF_INIT_VALUE && node->ref_count < 10);

  SysTypeClass *cls = node->data.v.class_ptr;

  sys_type_class_free(cls);
  sys_free_N(node->name);
  sys_free_N(node);
}

void sys_type_node_unref(SysTypeNode *node) {
  if (!sys_ref_count_dec(&node->data)) {
    return;
  }

  if (!sys_ref_count_dec(node)) {
    return;
  }

  node->data.v.class_finalize(node->data.v.class_ptr);
}

void *sys_type_get_private(SysTypeInstance *instance, SysType type) {
  SysTypeNode *info = sys_type_node(type);

  return (((SysChar *)instance) - info->data.v.private_size);
}

void sys_type_class_free(SysTypeClass *cls) {
	sys_return_if_fail(cls != NULL && "type class is null before free, useless type ?");

  SysTypeNode *node = sys_type_node(cls->type);

  if (cls != NULL) {
    sys_free_N(node->data.v.class_ptr);
  }
}

void sys_type_class_unref(SysTypeClass *cls) {
  SysTypeNode *node = sys_type_node(cls->type);

  assert(node->data.v.class_ptr == cls && "class unref not same as node");

  if (!sys_ref_count_dec(&(node->data))) {
    return;
  }

  sys_type_class_free(cls);
}

SysTypeClass *sys_type_class_ref(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  SysType ptype;
  SysTypeNode *node;
  SysTypeNode *pnode;
  SysTypeClass *cls, *pcls;

  node = sys_type_node(type);
  cls = node->data.v.class_ptr;

  sys_ref_count_inc(&(node->data));

  if(cls != NULL) {
    return cls;
  }

  cls = sys_malloc0_N(node->data.v.class_size);
  cls->type = (SysType)node;
  node->data.v.class_ptr = cls;

  for (SysInt i = node->n_supers; i >= 0; i--) {
    ptype = node->supers[i];
    pnode = sys_type_node(ptype);
    if(pnode == NULL) { continue; }

    pcls = sys_type_class_ref(ptype);
    pnode->data.v.class_init(cls);
    sys_type_class_unref(pcls);
  }

  return cls;
}

SysTypeInstance *sys_type_new_instance(SysType type) {
  SysTypeNode *pnode;
  SysTypeNode *node;
  int priv_psize = 0;
  SysTypeInstance *instance;
  SysChar *mp;
  SysTypeClass *cls;

  node = sys_type_node(type);
  priv_psize = node->data.v.private_size;

  mp = sys_malloc0_N(priv_psize + node->data.v.instance_size);
  instance = (SysTypeInstance *)(mp + priv_psize);

  cls = sys_type_class_ref(type);
  for (SysInt i = node->n_supers; i > 0; i--) {
    SysType p = node->supers[i];
    pnode = sys_type_node(p);
    pnode->data.v.instance_init(instance, cls);
  }

  instance->type_class = cls;
  node->data.v.instance_init(instance, cls);

  return instance;
}

void sys_type_free_instance(SysTypeInstance *instance) {
  SysTypeNode *node;
  SysTypeClass *cls;
  SysChar *real_ptr;

  cls = instance->type_class;
  node = sys_type_node(cls->type);

  real_ptr = ((SysChar*)instance) - node->data.v.private_size;
  sys_free_N(real_ptr);

  sys_type_class_unref(cls);
}

SysChar *sys_type_name(SysType type) {
  const SysTypeNode *info = sys_type_node(type);
  return info->name;
}

SysType sys_type_get_by_name(const SysChar *name) {
  assert(ht != NULL && "sys types should init before use.");

  return (SysType)sys_hash_table_lookup(ht, (const SysPointer)name);
}

SysTypeClass *sys_type_pclass(SysType type) {
  SysTypeNode *node, *pnode;
  SysType pType;

  node = sys_type_node(type); 
  pType = node->supers[node->n_supers];
  pnode = sys_type_node(pType);

  return (SysTypeClass *)pnode->data.v.class_ptr;
}

void sys_type_deinit(void) { 
  sys_hash_table_unref(ht); 
}

void sys_type_init(void) {
  sys_assert(ht == NULL && "type should init once.");

  ht = sys_hash_table_new_full(sys_str_hash, 
      (SysEqualFunc)sys_streq, 
      (SysDestroyFunc)sys_free, 
      (SysDestroyFunc)sys_type_node_free);
}

static inline SysTypeNode* sys_type_node(SysType type) {
  if (type == 0) {
    return NULL;
  }

  return (SysTypeNode *)type;
}
