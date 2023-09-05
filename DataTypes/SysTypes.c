#include <System/DataTypes/SysTypes.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/Utils/SysString.h>
#include <System/Platform/Common/SysThread.h>


#define NODE_TYPE(o) ((SysType)(o))
#define NODE_PARENT(node) (node->supers[1])
#define NODE_IS_ANCESTOR(ancestor, node)                                                    \
        ((ancestor)->n_supers <= (node)->n_supers &&                                        \
        (node)->supers[(node)->n_supers - (ancestor)->n_supers] == NODE_TYPE (ancestor))

typedef struct _InstanceData InstanceData;

struct _InstanceData {
  SysInt class_size;
  SysInt instance_size;
  SysInt private_size;
  SysTypeInitFunc class_init;  // set class_intern_init
  SysInstanceInitFunc instance_init;
  SysTypeFinalizeFunc class_finalize;
  void *class_ptr;
};

union _SysTypeData {
  InstanceData instance;
};

struct _SysTypeNode {
  SysChar *name;
  SysTypeData data;
  SysRef ref_count;

  SysInt n_supers;
  SysType supers[1]; // must be last field
};


static SysRefHook sys_object_unref_debug_func = NULL;
static SysRefHook sys_object_ref_debug_func = NULL;
static SysRWLock type_lock;

static SysHashTable *ht = NULL;

static void sys_object_base_class_init(SysObjectClass *self);
static void sys_object_base_finalize(SysObject *self);

static inline SysTypeNode* sys_type_node(SysType type);
static void sys_object_init(SysObject *self);
static void sys_object_class_init(SysObjectClass *self);

static SysTypeClass* sys_object_parent_class = NULL;
static SysInt SysObject_private_offset;
SysPointer sys_object_get_private(SysObject* o) {
  return (((SysUInt8 *)o) + SysObject_private_offset);
}
static void sys_object_class_intern_init(SysPointer cls) {
  sys_object_parent_class = NULL;
  sys_type_class_adjust_private_offset(cls, &SysObject_private_offset);
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
      "SysObject",

      (SysTypeInitFunc)sys_object_base_class_init,
      (SysDestroyFunc)sys_object_base_finalize,

      (SysTypeInitFunc)sys_object_class_intern_init,
      NULL,
      (SysInstanceInitFunc)sys_object_init
  };

  type = sys_type_new(0, &info);
  return type;
}

static void sys_object_base_finalize(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_dispose_i(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_finalize_i(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_base_class_init(SysObjectClass *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_class_init(SysObjectClass *ocls) {
  ocls->dispose = sys_object_dispose_i;
  ocls->finalize = sys_object_finalize_i;
}

static void sys_object_init(SysObject *self) {
}

void* sys_object_new(SysType type, const SysChar * first, ...) {
  SysObject *o;

  o = (SysObject *)sys_type_new_instance(type);
  if (o == NULL) { return NULL; }
  sys_ref_count_init(o);

  return o;
}

void _sys_object_ref(SysObject* self) {
  sys_return_if_fail(self != NULL);
  sys_assert(SYS_REF_CHECK(self, MAX_REF_NODE));

#if SYS_DEBUG

  if (sys_object_ref_debug_func) {
    SysType type = sys_type_from_instance(self);
    SysTypeNode *node = sys_type_node(type);
    sys_object_ref_debug_func(self, node->name, sys_atomic_int_get(&self->ref_count));
  }
#endif

  sys_ref_count_inc(self);
}

void _sys_object_unref(SysObject* self) {
  sys_return_if_fail(self != NULL);
  sys_assert(SYS_REF_CHECK(self, MAX_REF_NODE));

  SysObjectClass *cls;

#if SYS_DEBUG
  if (sys_object_unref_debug_func) {
    SysType type = sys_type_from_instance(self);
    SysTypeNode *node = sys_type_node(type);

    sys_object_unref_debug_func(self, node->name, sys_atomic_int_get(&self->ref_count));
  }
#endif

  if(!sys_ref_count_dec(self)) {
    return;
  }
  cls = SYS_OBJECT_GET_CLASS(self);

  if (cls->dispose) {
    cls->dispose(self);
  }

  sys_type_free_instance((SysTypeInstance *)self);
}

void sys_object_set_unref_hook(SysRefHook hook) {
  sys_object_unref_debug_func = hook;
}

void sys_object_set_ref_hook(SysRefHook hook) {
  sys_object_ref_debug_func = hook;
}

void * _sys_object_cast_check(SysObject* self, SysType ttype) {
  if (self == NULL) {
    return NULL;
  }

  SysType type = sys_type_from_instance(self);
  SysTypeNode *node = sys_type_node(type);

  if (sys_atomic_int_get(&node->ref_count) < 0
    || sys_atomic_int_get(&node->n_supers) < 0) {

    sys_error_N("%s", "Object check node Failed");
    return NULL;
  }

  if (!sys_ref_count_cmp(self, 0)) {
    sys_return_val_if_fail(sys_type_is_a(type, ttype), NULL);
    sys_return_val_if_fail(SYS_REF_CHECK(self, MAX_REF_NODE), NULL);
  }

  return self;
}

void* _sys_class_cast_check(SysObjectClass* cls, SysType ttype) {
  sys_return_val_if_fail(cls != NULL, NULL);

  sys_assert(cls->dispose != NULL);

  SysType type = sys_type_from_class(cls);
  SysTypeNode *node = sys_type_node(type);

  if (sys_atomic_int_get(&node->ref_count) < 0
    || sys_atomic_int_get(&node->n_supers) < 0
    || !sys_type_is_a(type, ttype)) {

    sys_error_N("%s", "Class check node Failed");
    return NULL;
  }

  return cls;
}

SysBool _sys_object_is_a(SysObject *self, SysType type) {
  SysType otype = sys_type_from_instance(self);

  return sys_type_is_a(otype, type);
}

/* SysType */
SysType sys_type_new(SysType ptype, const SysTypeInfo *info) {
  SysTypeNode *pnode;
  SysTypeNode *node;
  int nodesize = 0;
  int pn_supers = 0;

  pnode = sys_type_node(ptype);

  if (pnode) {
    pn_supers = pnode->n_supers + 1;
  }

  nodesize = (int)sizeof(SysTypeNode) + (int)sizeof(SysType) * (pn_supers + 1);
  node = sys_malloc0_N(nodesize);
  node->name = sys_strdup(info->name);

  node->data.instance.instance_size = info->instance_size;
  node->data.instance.class_size = info->class_size;
  node->data.instance.class_init = info->class_init;
  node->data.instance.class_finalize = info->class_finalize;
  node->data.instance.instance_init = info->instance_init;
  node->n_supers = pn_supers;

  node->supers[0] = (SysType)node;
  if (pnode) {
    sys_memcpy(node->supers + 1, sizeof(SysType) * (node->n_supers), pnode->supers, sizeof(SysType) * node->n_supers);
  } else {
    node->supers[1] = 0;
  }

  sys_ref_count_init(node);

  assert(ht != NULL && "sys types should be initiated before use.");

  sys_hash_table_insert(ht, node->name, node);

  return (SysType)node;
}

void sys_type_node_free(SysTypeNode *node) {
  SysTypeClass *cls = node->data.instance.class_ptr;

  if(cls != NULL) {
    sys_type_class_free(cls);
  } else {
    sys_debug_N("type class is null, maybe useless type: \"%s\" ?", node->name);
  }

  sys_free_N(node->name);
  sys_free_N(node);
}

void sys_type_node_unref(SysTypeNode *node) {
  sys_assert(SYS_REF_CHECK(node, MAX_REF_NODE));

  sys_type_node_free(node);
}

void *sys_type_get_private(SysTypeInstance *instance, SysType type) {
  SysTypeNode *node = sys_type_node(type);

  return ((SysUInt8 *)instance) - node->data.instance.private_size;
}

void sys_type_class_adjust_private_offset (SysTypeClass *cls, SysInt * private_offset) {
  sys_return_if_fail(cls != NULL);
  sys_return_if_fail(private_offset != NULL);

  SysTypeNode* node = sys_type_node(cls->type);
  SysTypeNode* pnode = sys_type_node(NODE_PARENT(node));

  sys_rw_lock_writer_lock(&type_lock);

  if (*private_offset >= 0) {
    sys_return_if_fail (*private_offset <= 0xffff);

    sys_assert(node->data.instance.private_size <= 0xffff);
    node->data.instance.private_size = pnode ? pnode->data.instance.private_size + *private_offset : *private_offset;
    *private_offset = -(SysInt)node->data.instance.private_size;
  }

  sys_rw_lock_writer_unlock(&type_lock);
}

void sys_type_class_free(SysTypeClass *cls) {
  sys_return_if_fail(cls != NULL);

  SysTypeNode *node = sys_type_node(cls->type);

  if (node->data.instance.class_finalize) {
    node->data.instance.class_finalize(node->data.instance.class_ptr);
  }

  if (cls != NULL) {
    sys_free_N(node->data.instance.class_ptr);
  }
}

void sys_type_class_unref(SysTypeClass *cls) {
  SysTypeNode *node = sys_type_node(cls->type);

  if (!sys_ref_count_dec(node)) {
    return;
  }

  sys_type_class_free(cls);
}

SysTypeClass *sys_type_class_ref(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  SysTypeNode *node;
  SysTypeNode *pnode;
  SysTypeClass *cls, *pcls;

  node = sys_type_node(type);

  cls = node->data.instance.class_ptr;

  sys_ref_count_inc(node);

  if(cls != NULL) {
    return cls;
  }

  cls = sys_malloc0_N(node->data.instance.class_size);
  cls->type = (SysType)node;
  node->data.instance.class_ptr = cls;

  for (SysInt i = node->n_supers; i > 0; i--) {
    SysType ptype = node->supers[i];
    pnode = sys_type_node(ptype);

    pcls = sys_type_class_ref(ptype);
    pnode->data.instance.class_init(cls);
    sys_type_class_unref(pcls);
  }
  node->data.instance.class_init(cls);

  sys_assert(node->data.instance.private_size >= 0 && "check class inherit and GET_CLASS OR CLASS use wrong?.");

  return cls;
}

SysTypeInstance *sys_type_new_instance(SysType type) {
  SysTypeNode *pnode;
  SysTypeNode *node;
  SysTypeInstance *instance;
  SysChar *mp;
  SysTypeClass *cls;
  SysInstanceInitFunc ofunc, nfunc;

  int priv_psize = 0;

  node = sys_type_node(type);

  ofunc = node->data.instance.instance_init;
  cls = sys_type_class_ref(type);
  nfunc = node->data.instance.instance_init;

  sys_assert(ofunc == nfunc && "maybe use *_CLASS instead of *_GET_CLASS ? ");

  priv_psize = node->data.instance.private_size;

  mp = sys_malloc0_N(priv_psize + node->data.instance.instance_size);
  instance = (SysTypeInstance *)(mp + priv_psize);
  instance->type_class = cls;

  for (SysInt i = node->n_supers; i > 0; i--) {
    SysType p = node->supers[i];
    pnode = sys_type_node(p);

    pnode->data.instance.instance_init(instance);
  }

  node->data.instance.instance_init(instance);

  return instance;
}

void sys_type_free_instance(SysTypeInstance *instance) {
  SysTypeNode *node;
  SysTypeClass *cls;
  SysChar *real_ptr;

  cls = instance->type_class;
  node = sys_type_node(cls->type);

  real_ptr = ((SysChar*)instance) - node->data.instance.private_size;
  sys_free_N(real_ptr);

  sys_type_class_unref(cls);
}

SysChar *sys_type_name(SysType type) {
  const SysTypeNode *info = sys_type_node(type);
  return info->name;
}

SysType sys_type_get_by_name(const SysChar *name) {
  assert(ht != NULL && "sys types should be initiated before use.");

  return (SysType)sys_hash_table_lookup(ht, (const SysPointer)name);
}

SysTypeClass *sys_type_pclass(SysType type) {
  SysTypeNode *node, *pnode;
  SysType pType;

  node = sys_type_node(type);
  pType = node->supers[node->n_supers];
  pnode = sys_type_node(pType);

  return (SysTypeClass *)pnode->data.instance.class_ptr;
}

void sys_type_teardown(void) {
  sys_hash_table_unref(ht);
  ht = NULL;

  sys_rw_lock_clear(&type_lock);
}

void sys_type_setup(void) {
  sys_assert(ht == NULL && "type should be initiated once.");

  ht = sys_hash_table_new_full(sys_str_hash,
      (SysEqualFunc)sys_str_equal,
      NULL,
      (SysDestroyFunc)sys_type_node_unref);

  sys_rw_lock_init(&type_lock);
}

static SYS_INLINE SysTypeNode* sys_type_node(SysType type) {
  if (type == 0) {
    return NULL;
  }

  return (SysTypeNode *)type;
}

SysBool sys_type_is_a(SysType child, SysType parent) {
  sys_return_val_if_fail(child != 0, false);

  SysTypeNode *ancestor = sys_type_node(parent);
  SysTypeNode *node = sys_type_node(child);

  return NODE_IS_ANCESTOR(ancestor, node);
}
