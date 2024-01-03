#include <System/DataTypes/SysTypes.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/Utils/SysString.h>
#include <System/Platform/Common/SysThread.h>


#define NODE_TYPE(node) (node->supers[0])
#define NODE_PARENT(node) (node->supers[1])
#define NODE_IS_ANCESTOR(ancestor, node)                                                    \
        ((ancestor)->n_supers <= (node)->n_supers &&                                        \
        (node)->supers[(node)->n_supers - (ancestor)->n_supers] == NODE_TYPE (ancestor))
#define NODE_IS_IFACE(node)   (node->supers[node->n_supers] == SYS_TYPE_INTERFACE)
#define NODE_FUNDAMENTAL_TYPE(node)		(node->supers[node->n_supers])

typedef struct _InstanceData InstanceData;
typedef struct _IFaceData IFaceData;
typedef union _TypeData TypeData;

struct _InstanceData {
  SysInt class_size;
  SysInt instance_size;
  SysInt private_size;
  SysTypeInitFunc class_init;  // set class_intern_init
  SysInstanceInitFunc instance_init;
  SysTypeFinalizeFunc class_finalize;
  void* class_ptr;

  SysUInt16 n_ifaces;
  SysTypeInterface** ifaces;
};

struct _IFaceData {
  SysTypeInitFunc vtable_init;

  SysUInt16 vtable_size;
};

union _TypeData {
  InstanceData instance;
  IFaceData iface;
};

struct _TypeNode {
  SysChar* name;
  SysRef ref_count;

  SysUInt node_type;
  TypeData data;

  SysInt n_supers;
  SysType supers[1]; // must be the last field
};

static SysRefHook sys_object_unref_debug_func = NULL;
static SysRefHook sys_object_ref_debug_func = NULL;
static SysRefHook sys_object_new_debug_func = NULL;

static SysRWLock type_rw_lock;
static SysRecMutex class_init_rec_mutex;

static SysHashTable* ht = NULL;
static TypeNode* static_fundamental_type_nodes[(SYS_TYPE_FUNDAMENTAL_MAX >> SYS_TYPE_FUNDAMENTAL_SHIFT) + 1] = { NULL, };
TypeNode * sys_make_fundamental_node(const TypeNode * pnode, SysType ftype, const SysTypeInfo * info);

static void sys_object_base_class_init(SysObjectClass* self);
static void sys_object_base_finalize(SysObject* self);

static inline TypeNode* sys_type_node(SysType type);
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

  sys_make_fundamental_node(NULL, SYS_TYPE_OBJECT, &type_info);
}

static void interface_default_init(SysTypeInterface* iface) {
}

void _sys_interface_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_INTERFACE,
    sizeof(SysTypeInterface),
    0,
    "SysInterface",

    NULL,
    NULL,

    (SysInterfaceInitFunc)interface_default_init,
    NULL,
    NULL
  };

  sys_make_fundamental_node(NULL, SYS_TYPE_INTERFACE, &type_info);
}

static void sys_object_base_finalize(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

static void sys_object_dispose_i(SysObject *self) {
  sys_return_if_fail(self != NULL);
}

SysObject* _sys_object_new_from_instance(SysObject *o) {
  sys_return_val_if_fail(o != NULL, NULL);

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

static void sys_object_class_init(SysObjectClass *ocls) {
  ocls->dclone = sys_object_dclone_i;
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


#if SYS_DEBUG
  if (sys_object_new_debug_func) {
    TypeNode *node = sys_type_node(type);
    sys_object_new_debug_func(o, node->name, sys_atomic_int_get(&o->ref_count));
  }
#endif

  return o;
}

SysPointer _sys_object_ref(SysObject* self) {
  sys_return_val_if_fail(self != NULL, NULL);
  sys_return_val_if_fail(SYS_REF_CHECK(self, MAX_REF_NODE), NULL);

#if SYS_DEBUG

  if (sys_object_ref_debug_func) {
    SysType type = sys_type_from_instance(self);
    TypeNode *node = sys_type_node(type);
    sys_object_ref_debug_func(self, node->name, sys_atomic_int_get(&self->ref_count));
  }
#endif

  sys_ref_count_inc(self);

  return (SysPointer)self;
}

void _sys_object_unref(SysObject* self) {
  sys_return_if_fail(self != NULL);

  SysObjectClass* cls;

  if(!SYS_REF_VALID_CHECK(self, MAX_REF_NODE)) {
    sys_warning_N("object ref check failed: %p", self);
    return;
  }

#if SYS_DEBUG
  if (sys_object_unref_debug_func) {
    SysType type;
    TypeNode* node;

    type = sys_type_from_instance(self);
    node = sys_type_node(type);

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

void sys_object_set_new_hook(SysRefHook hook) {
  sys_object_new_debug_func = hook;
}

void * _sys_object_cast_check(SysObject* self, SysType ttype) {
  if (self == NULL) {
    return NULL;
  }

  SysType type = sys_type_from_instance(self);
  TypeNode *node = sys_type_node(type);
  TypeNode* tnode;

  if (sys_atomic_int_get(&node->ref_count) < 0
    || sys_atomic_int_get(&node->n_supers) < 0) {

    sys_error_N("%s", "Object check node Failed");
    return NULL;
  }

  if (!sys_ref_count_cmp(self, 0)) {
    tnode = sys_type_node(ttype);

    if (tnode->node_type & SYS_NODE_INTERFACE) {

    } else {

      if (!sys_type_is_a(type, ttype)) {
        sys_error_N("Object check node failed: %p, %s to %s", self, node->name, tnode->name);
        return NULL;
      }

    }

    sys_return_val_if_fail(SYS_REF_CHECK(self, MAX_REF_NODE), NULL);
  }

  return self;
}

void* _sys_class_cast_check(SysObjectClass* cls, SysType ttype) {
  sys_return_val_if_fail(cls != NULL, NULL);
  sys_return_val_if_fail(cls->dispose != NULL, NULL);

  SysType type = sys_type_from_class(cls);
  TypeNode* node = sys_type_node(type);
  TypeNode* tnode;

  if (sys_atomic_int_get(&node->ref_count) < 0
    || sys_atomic_int_get(&node->n_supers) < 0
    || !sys_type_is_a(type, ttype)) {
    tnode = sys_type_node(ttype);

    sys_error_N("Class check node Failed: %s to %s", node->name, tnode->name);
    return NULL;
  }

  return cls;
}

SysBool _sys_object_is_a(SysObject *self, SysType type) {
  sys_return_val_if_fail(SYS_REF_CHECK(self, MAX_REF_NODE), false);

  SysType otype = sys_type_from_instance(self);

  return sys_type_is_a(otype, type);
}

const SysChar* _sys_object_get_type_name(SysObject *self) {
  sys_return_val_if_fail(self != NULL, NULL);
  sys_return_val_if_fail(SYS_REF_CHECK(self, MAX_REF_NODE), NULL);

  SysType tp = sys_type_from_instance(self);
  TypeNode* node = sys_type_node(tp);

  return node->name;
}

/* SysType */
void sys_type_ht_insert(TypeNode *node) {
  sys_return_if_fail(node->name != NULL);

  assert(ht != NULL && "sys types should be initiated before use.");

  sys_hash_table_insert(ht, node->name, node);
}

void sys_type_ht_remove(TypeNode* node) {
  sys_hash_table_remove(ht, (SysPointer)node->name);
}

TypeNode* sys_type_make_node(const TypeNode* pnode, const SysTypeInfo *info, SysInt flags) {
  TypeNode* node;
  SysInt nodesize = 0;
  SysInt pn_supers = 0;

  if (pnode) {
    pn_supers = pnode->n_supers + 1;
  }

  if (info->node_type & SYS_NODE_CLASS) {
    if (info->class_size <= 0) {
      sys_warning_N("class size must bigger than zero: %s", info->name);
      return NULL;
    }

    if (info->class_init == NULL) {
      sys_warning_N("class init must set: %s", info->name);
      return NULL;
    }
  }

  nodesize = (SysInt)sizeof(TypeNode) + (SysInt)sizeof(SysType) * (pn_supers + 1);
  node = sys_malloc0_N(nodesize);
  node->node_type = info->node_type;
  node->name = sys_strdup(info->name);
  node->n_supers = pn_supers;
  node->supers[0] = (SysType)node;

  if (pnode) {
    sys_memcpy(node->supers + 1, sizeof(SysType) * (node->n_supers), pnode->supers, sizeof(SysType) * node->n_supers);

  } else {

    node->supers[1] = 0;
  }

  node->node_type = info->node_type;
  switch (info->node_type) {
    case SYS_NODE_FUNDAMENTAL:
      break;
    case SYS_NODE_ABSTRACT_CLASS:
      break;
    case SYS_NODE_CLASS:
    case SYS_NODE_BASE_CLASS:
      node->data.instance.instance_size = info->instance_size;
      node->data.instance.class_size = info->class_size;
      node->data.instance.class_init = info->class_init;
      node->data.instance.class_finalize = info->class_finalize;
      node->data.instance.instance_init = info->instance_init;
      break;
    case SYS_NODE_INTERFACE:
      node->data.iface.vtable_init = info->class_init;
      node->data.iface.vtable_size = info->class_size;
      break;
    default:
      sys_abort_N("Not correct declare type for node: %s", info->name);
      break;
  }

  sys_ref_count_init(node);

  return node;
}

TypeNode * sys_make_fundamental_node(const TypeNode * pnode, SysType ftype, const SysTypeInfo * info) {
  sys_rw_lock_writer_lock(&type_rw_lock);

  TypeNode* node = sys_type_make_node(pnode, info, 0);
  static_fundamental_type_nodes[ftype >> SYS_TYPE_FUNDAMENTAL_SHIFT] = node;
  node->supers[0] = ftype;
  sys_type_ht_insert(node);

  sys_rw_lock_writer_unlock(&type_rw_lock);

  return node;
}

void _sys_fnode_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    0,
    "SysFNode",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_make_fundamental_node(NULL, SYS_TYPE_FNODE, &type_info);
}

SysType sys_type_new(SysType ptype, const SysTypeInfo *info) {
  sys_return_val_if_fail(info != NULL, 0);

  sys_rw_lock_writer_lock(&type_rw_lock);

  TypeNode* pnode = sys_type_node(ptype);
  TypeNode* node = sys_type_make_node(pnode, info, 0);
  if (node == NULL) {
    return 0;
  }

  sys_type_ht_insert(node);

  sys_rw_lock_writer_unlock(&type_rw_lock);

  return (SysType)node;
}

void sys_type_node_free(TypeNode *node) {
  SysTypeClass *cls;

  switch (node->node_type) {
    case SYS_NODE_FUNDAMENTAL:
      break;
    case SYS_NODE_ABSTRACT_CLASS:
    case SYS_NODE_CLASS:
    case SYS_NODE_BASE_CLASS:
      cls = node->data.instance.class_ptr;

      if (node->data.instance.n_ifaces > 0) {
        for (SysInt i = 0; i < node->data.instance.n_ifaces; i++) {

          sys_clear_pointer(&(node->data.instance.ifaces[i]->vtable_ptr), sys_free);
        }

        sys_clear_pointer(&node->data.instance.ifaces, sys_free);
      }

      if (cls != NULL) {
        sys_type_class_free(cls);
      }
      else {
        sys_debug_N("type class is null, maybe useless type: \"%s\" ?", node->name);
      }
      break;
    case SYS_NODE_INTERFACE:
      break;
    default:
      sys_abort_N("Not correct declare type when free node: %s", node->name);
      break;
  }

  sys_free_N(node->name);
  sys_free_N(node);
}

void sys_type_node_unref(TypeNode *node) {
  sys_assert(SYS_REF_CHECK(node, MAX_REF_NODE));

  sys_type_node_free(node);
}

void *sys_type_get_private(SysTypeInstance *instance, SysType type) {
  TypeNode *node = sys_type_node(type);

  return ((SysUInt8 *)instance) - node->data.instance.private_size;
}

void sys_type_class_adjust_private_offset (SysTypeClass *cls, SysInt * private_offset) {
  sys_return_if_fail(cls != NULL);
  sys_return_if_fail(private_offset != NULL);

  TypeNode* node = sys_type_node(cls->type);
  TypeNode* pnode = sys_type_node(NODE_PARENT(node));

  sys_rw_lock_writer_lock(&type_rw_lock);

  if (*private_offset >= 0) {
    sys_return_if_fail (*private_offset <= 0xffff);

    sys_assert(node->data.instance.private_size <= 0xffff);
    node->data.instance.private_size = pnode ? pnode->data.instance.private_size + *private_offset : *private_offset;
    *private_offset = -(SysInt)node->data.instance.private_size;
  }

  sys_rw_lock_writer_unlock(&type_rw_lock);
}

void sys_type_class_free(SysTypeClass *cls) {
  sys_return_if_fail(cls != NULL);

  TypeNode *node = sys_type_node(cls->type);

  if (node->data.instance.class_finalize) {
    node->data.instance.class_finalize(node->data.instance.class_ptr);
  }

  if (cls != NULL) {
    sys_free_N(node->data.instance.class_ptr);
  }
}

void sys_type_class_unref(SysTypeClass *cls) {
  TypeNode *node = sys_type_node(cls->type);

  if (!sys_ref_count_dec(node)) {
    return;
  }

  sys_type_class_free(cls);
}

SysTypeClass *sys_type_class_ref(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  TypeNode *node;
  TypeNode *pnode;
  SysTypeClass *cls, *pcls;

  node = sys_type_node(type);

  cls = node->data.instance.class_ptr;

  sys_ref_count_inc(node);

  if(cls != NULL) {
    return cls;
  }

  switch (node->node_type) {
    case SYS_NODE_FUNDAMENTAL:
      break;
    case SYS_NODE_ABSTRACT_CLASS:
      break;
    case SYS_NODE_CLASS:
    case SYS_NODE_BASE_CLASS:
      cls = sys_malloc0_N(node->data.instance.class_size);
      cls->type = (SysType)node;
      node->data.instance.class_ptr = cls;

      for (SysInt i = node->n_supers; i > 0; i--) {
        SysType ptype = node->supers[i];
        pnode = sys_type_node(ptype);

        pcls = sys_type_class_ref(ptype);
        if (pcls) {
          pnode->data.instance.class_init(cls);
          sys_type_class_unref(pcls);
        }
      }
      node->data.instance.class_init(cls);

      sys_assert(node->data.instance.private_size >= 0 && "check class inherit and GET_CLASS OR CLASS use wrong?.");
      break;
    case SYS_NODE_INTERFACE:
      break;
    default:
      sys_abort_N("Not correct declare type for node: %s", node->name);
      break;
  }

  return cls;
}

SysTypeInstance *sys_type_new_instance(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  TypeNode *pnode;
  TypeNode *node;
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

    if (pnode->node_type & SYS_NODE_CLASS) {
      pnode->data.instance.instance_init(instance);
    }
  }

  node->data.instance.instance_init(instance);

  return instance;
}

void sys_type_free_instance(SysTypeInstance *instance) {
  sys_return_if_fail(instance != NULL);

  TypeNode *node;
  SysTypeClass *cls;
  SysChar *real_ptr;

  cls = instance->type_class;
  node = sys_type_node(cls->type);

  real_ptr = ((SysChar*)instance) - node->data.instance.private_size;
  sys_free_N(real_ptr);

  sys_type_class_unref(cls);
}

SysChar *sys_type_name(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  const TypeNode *info = sys_type_node(type);
  return info->name;
}

SysType sys_type_get_by_name(const SysChar *name) {
  assert(ht != NULL && "sys types should be initiated before use.");

  return (SysType)sys_hash_table_lookup(ht, (const SysPointer)name);
}

SysTypeClass *sys_type_pclass(SysType type) {
  TypeNode *node, *pnode;
  SysType pType;

  node = sys_type_node(type);
  pType = node->supers[node->n_supers];
  pnode = sys_type_node(pType);

  return (SysTypeClass *)pnode->data.instance.class_ptr;
}

void sys_type_teardown(void) {
  sys_hash_table_unref(ht);
  ht = NULL;

  sys_rw_lock_clear(&type_rw_lock);
  sys_rec_mutex_clear(&class_init_rec_mutex);
}

void sys_type_setup(void) {
  sys_assert(ht == NULL && "type should be initiated once.");

  sys_rw_lock_init(&type_rw_lock);
  sys_rec_mutex_init(&class_init_rec_mutex);

  ht = sys_hash_table_new_full(sys_str_hash,
      (SysEqualFunc)sys_str_equal,
      NULL,
      (SysDestroyFunc)sys_type_node_unref);

  _sys_fnode_init_type();
  _sys_interface_init_type();
  _sys_object_init_type();
}

static SYS_INLINE TypeNode* sys_type_node(SysType utype) {
  if (utype > SYS_TYPE_FUNDAMENTAL_MAX)
    return (TypeNode*)(utype & ~TYPE_ID_MASK);
  else
    return static_fundamental_type_nodes[utype >> SYS_TYPE_FUNDAMENTAL_SHIFT];
}

SysBool sys_type_is_a(SysType child, SysType parent) {
  sys_return_val_if_fail(child != 0, false);

  TypeNode *ancestor = sys_type_node(parent);
  TypeNode *node = sys_type_node(child);

  return NODE_IS_ANCESTOR(ancestor, node);
}

SysTypeInterface* sys_type_instance_get_iface(TypeNode *node, SysType iface_type) {
  InstanceData *o = &node->data.instance;
  for (SysUInt i = 0; i < o->n_ifaces; i++) {
    SysTypeInterface *iface = o->ifaces[i];

    if (iface->type == iface_type) {
      return iface;
    }
  }

  return NULL;
}

SysTypeInterface* _sys_type_get_interface(SysTypeClass *cls, SysType iface_type) {
  sys_return_val_if_fail(cls != 0, NULL);

  TypeNode *node = sys_type_node(cls->type);
  TypeNode *face_node = sys_type_node(iface_type);

  sys_return_val_if_fail(NODE_IS_IFACE(face_node), NULL);

  return sys_type_instance_get_iface(node, iface_type);
}

/**
 * simple implementation
 */
void sys_type_imp_interface(SysType instance_type, SysType iface_type, const SysInterfaceInfo *info) {
  sys_return_if_fail(instance_type > 0);
  sys_return_if_fail(iface_type > 0);
  sys_return_if_fail(info != NULL);

  sys_rec_mutex_lock(&class_init_rec_mutex);
  sys_rw_lock_writer_lock (&type_rw_lock);

  SysTypeInterface* iface;
  IFaceData* iface_data;
  InstanceData *instance_data;

  TypeNode *node = sys_type_node (instance_type);
  if (node == NULL || !(node->node_type & SYS_NODE_CLASS)) {
    sys_warning_N("instance type node not found when implement interface: %d", instance_type);
  }

  TypeNode *iface_node = sys_type_node (iface_type);
  if (iface_node == NULL || !(iface_node->node_type & SYS_NODE_INTERFACE)) {
    sys_abort_N("interface node not found when implement interface: %d", iface_type);
  }

  instance_data = &node->data.instance;
  iface_data = &iface_node->data.iface;

  if(!info->interface_init) {
    sys_abort_N("interface init function required: %s,%s", node->name, iface_node->name);
  }

  if (iface_data->vtable_size < sizeof(SysTypeInterface)) {
    sys_abort_N("interface must inherit SysTypeInterface: %s,%s", node->name, iface_node->name);
  }

  iface = sys_malloc0_N(iface_data->vtable_size);
  iface->instance_type = instance_type;
  iface->type = iface_type;

  iface_data->vtable_init(iface);
  info->interface_init(iface);
  iface->vtable_ptr = iface;

  SysTypeInterface** nmem = sys_new0_N(SysTypeInterface *, instance_data->n_ifaces + 1);
  if (instance_data->n_ifaces > 0) {
    sys_memcpy (
      nmem + 1, sizeof(SysTypeInterface*) * instance_data->n_ifaces,
      instance_data->ifaces, sizeof(SysTypeInterface*) * instance_data->n_ifaces);
    sys_clear_pointer(&instance_data->ifaces, sys_free);
  }

  nmem[0] = iface;
  instance_data->n_ifaces += 1;
  instance_data->ifaces = nmem;

  sys_rw_lock_writer_unlock(&type_rw_lock);
  sys_rec_mutex_unlock(&class_init_rec_mutex);
}
