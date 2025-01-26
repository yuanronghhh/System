#include <System/Type/SysType.h>
#include <System/Type/SysObject.h>
#include <System/Type/SysParam.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/DataTypes/SysHArray.h>
#include <System/DataTypes/SysSList.h>
#include <System/Utils/SysStr.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysRefCount.h>

#define NODE_TYPE(node) (node->supers[0])
#define NODE_PARENT(node) (node->supers[1])
#define NODE_IS_ANCESTOR(ancestor, node)                                                    \
        ((ancestor)->n_supers <= (node)->n_supers &&                                        \
        (node)->supers[(node)->n_supers - (ancestor)->n_supers] == NODE_TYPE (ancestor))
#define NODE_IS_IFACE(node)   (node->supers[node->n_supers] == SYS_TYPE_INTERFACE)
#define NODE_FUNDAMENTAL_TYPE(node)  (node->supers[node->n_supers])

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
  SysHArray props;
  void* class_ptr;
};

struct _IFaceData {
  SysTypeInitFunc vtable_init;

  SysUInt16 vtable_size;
};

union _TypeData {
  InstanceData instance;
  IFaceData iface;
};

struct _SysTypeNode {
  SysChar* name;

  SysUInt node_type;
  TypeData data;
  SysRef ref_count;

  SysUInt n_supers;
  SysType supers[1]; // must be the last field
};

/* interface entry store on instance */
struct  _IFaceEntry {
  SysType iface_type;
  SysType instance_type;
  /* pointer to interface on class */
  SysTypeInterface* iface_ptr;
};

static SysRWLock type_rw_lock;
static SysMutex param_lock;
static SysRecMutex class_init_rec_mutex;

static SysHashTable* ht = NULL;
static SysSList *g_iface_entries = NULL;

static SysTypeNode* static_fundamental_type_nodes[(SYS_TYPE_FUNDAMENTAL_MAX >> SYS_TYPE_FUNDAMENTAL_SHIFT) + 1] = { NULL, };

static void interface_default_init(SysTypeInterface* iface) {
}

/* props */
void sys_type_prop_set(SysType tp, SysParam *param) {
  SysHArray *parray;

  sys_mutex_lock(&param_lock);
  SysTypeNode *node = sys_type_node(tp);
  if (node == NULL) {
    sys_warning_N("Not found SysTypeNode: %p", tp);
    return;
  }

  parray = &node->data.instance.props;
  sys_harray_add(parray, param);

  sys_mutex_unlock(&param_lock);
}

SysParam* sys_type_prop_get(SysType tp, const SysChar *name) {
  sys_return_val_if_fail(tp != 0, NULL);
  sys_return_val_if_fail(name != NULL, NULL);

  SysHArray *parray;
  SysParam *param = NULL;

  sys_mutex_lock(&param_lock);

  SysTypeNode *node = sys_type_node(tp);
  if (node == NULL) {
    sys_warning_N("Not found SysTypeNode: %p", tp);
    return NULL;
  }
  parray = &node->data.instance.props;
  for(SysUInt i = 0; i < parray->len; i++) {
    param = parray->pdata[i];
    const SysChar *nname = sys_param_get_field_name(param);

    if(sys_str_equal(nname, name)) {
      break;
    }
  }

  sys_mutex_unlock(&param_lock);

  return param;
}

SysHArray *sys_type_get_properties(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);

  SysTypeNode *node = sys_type_node(type);
  if (node == NULL) {
    sys_warning_N("Not found SysTypeNode: %p", type);
    return NULL;
  }

  return &node->data.instance.props;
}

/* SysType */
static void sys_type_ht_insert(SysTypeNode *node) {
  sys_return_if_fail(node->name != NULL);

  sys_assert(ht != NULL && "sys types should be initiated before use.");

  sys_hash_table_insert(ht, node->name, node);
}

static SysTypeNode* sys_type_make_node(const SysTypeNode* pnode,
    const SysTypeInfo *info,
    SysInt flags) {
  SysTypeNode* node;
  SysInt nodesize = 0;
  SysUInt pn_supers = 0;

  if (pnode) {

    pn_supers = pnode->n_supers + 1;
  }

  nodesize = (SysInt)sizeof(SysTypeNode) + (SysInt)sizeof(SysType) * (pn_supers + 1);

  node = sys_malloc(nodesize);
  sys_ref_count_init(node);

  node->node_type = info->node_type;
  node->name = sys_strdup(info->name);
  node->n_supers = pn_supers;
  node->supers[0] = (SysType)node;

  if (pnode) {
    sys_memcpy(node->supers + 1,
        sizeof(SysType) * (node->n_supers), 
        pnode->supers, 
        sizeof(SysType) * node->n_supers);

  } else {

    node->supers[1] = 0;
  }

  node->node_type = info->node_type;
  switch (info->node_type) {
    case SYS_NODE_FUNDAMENTAL:
      node->data.instance.instance_size = info->instance_size;
      node->data.instance.class_size = info->class_size;
      break;
    case SYS_NODE_ABSTRACT_CLASS:
    case SYS_NODE_CLASS:
    case SYS_NODE_BASE_CLASS:
      if (info->class_size <= 0) {
        sys_warning_N("class size must bigger than zero: %s", info->name);
        return NULL;
      }

      if (info->class_init == NULL) {
        sys_warning_N("class init must set: %s", info->name);
        return NULL;
      }

      node->data.instance.instance_size = info->instance_size;
      node->data.instance.class_size = info->class_size;
      node->data.instance.class_init = info->class_init;
      node->data.instance.class_finalize = info->class_finalize;
      node->data.instance.instance_init = info->instance_init;
      sys_harray_init_with_free_func(&node->data.instance.props,
          (SysDestroyFunc)_sys_object_unref);
      break;
    case SYS_NODE_INTERFACE:
      if (info->class_size < sizeof(SysTypeInterface)) {
        sys_warning_N("interface must inherit SysTypeInterface: %s", info->name);
        return NULL;
      }

      if (info->class_init == NULL) {
        sys_warning_N("interface init must set: %s", info->name);
        return NULL;
      }

      node->data.iface.vtable_init = info->class_init;
      node->data.iface.vtable_size = info->class_size;
      break;
    default:
      sys_abort_N("Not correct declare type for node: %s", info->name);
      break;
  }

  return node;
}

SysTypeNode * sys_type_make_fundamental_node(const SysTypeNode * pnode,
    SysType ftype, 
    const SysTypeInfo * info) {
  sys_rw_lock_writer_lock(&type_rw_lock);

  SysTypeNode* node = sys_type_make_node(pnode, info, 0);
  static_fundamental_type_nodes[ftype >> SYS_TYPE_FUNDAMENTAL_SHIFT] = node;
  node->supers[0] = ftype;
  sys_type_ht_insert(node);

  sys_rw_lock_writer_unlock(&type_rw_lock);

  return node;
}

static void _sys_interface_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_INTERFACE,
    sizeof(SysTypeInterface),
    0,
    "SysInterface",

    NULL,
    NULL,

    (SysTypeInitFunc)interface_default_init,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_INTERFACE, &type_info);
}

static void _sys_fundanmental_node_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    0,
    "SysFundanmentalNode",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_FUNDAMENTAL_NODE, &type_info);
}

static void _sys_char_node_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    sizeof(SysChar),
    "SysChar",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_CHAR, &type_info);
}

static void _sys_double_node_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    sizeof(SysDouble),
    "SysDouble",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_DOUBLE, &type_info);
}

static void _sys_bool_node_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    sizeof(SysBool),
    "SysBool",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_BOOL, &type_info);
}

static void _sys_int_node_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    sizeof(SysInt),
    "SysInt",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_INT, &type_info);
}

static void _sys_pointer_node_init_type(void) {
  const SysTypeInfo type_info = {
    SYS_NODE_FUNDAMENTAL,
    0,
    sizeof(SysPointer),
    "SysPointer",
    NULL,
    NULL,

    NULL,
    NULL,
    NULL
  };

  sys_type_make_fundamental_node(NULL, SYS_TYPE_POINTER, &type_info);
}

SysType sys_type_new(SysType ptype, const SysTypeInfo *info) {
  sys_return_val_if_fail(info != NULL, 0);

  sys_rw_lock_writer_lock(&type_rw_lock);

  SysTypeNode* pnode = sys_type_node(ptype);
  SysTypeNode* node = sys_type_make_node(pnode, info, 0);
  if (node == NULL) {
    return 0;
  }

  sys_type_ht_insert(node);

  sys_rw_lock_writer_unlock(&type_rw_lock);

  return (SysType)node;
}

void sys_type_node_free(SysTypeNode *node) {
  SysTypeClass *cls;

  switch (node->node_type) {
    case SYS_NODE_FUNDAMENTAL:
      break;
    case SYS_NODE_ABSTRACT_CLASS:
    case SYS_NODE_CLASS:
    case SYS_NODE_BASE_CLASS:
      cls = node->data.instance.class_ptr;

      if (node->data.instance.props.len > 0) {

        sys_harray_destroy(&node->data.instance.props);
      }

      if (cls != NULL) {
        sys_type_class_free(cls);
      } else {
        // sys_debug_N("type class is null, maybe useless type: \"%s\" ?", node->name);
      }
      break;
    case SYS_NODE_INTERFACE:
      break;
    default:
      sys_abort_N("Not correct declare type when free node: %s", node->name);
      break;
  }

  sys_free(node->name);
  sys_free(node);
}

void sys_type_node_unref(SysTypeNode *node) {

  sys_type_node_free(node);
}

void *sys_type_get_private(SysTypeInstance *instance, SysType type) {
  SysTypeNode *node = sys_type_node(type);

  return ((SysUInt8 *)instance) - node->data.instance.private_size;
}

void sys_type_class_adjust_private_offset (SysTypeClass *cls,
    SysInt * private_offset) {
  sys_return_if_fail(cls != NULL);
  sys_return_if_fail(private_offset != NULL);

  SysTypeNode* node = sys_type_node(sys_type_from_class(cls));
  SysTypeNode* pnode = sys_type_node(NODE_PARENT(node));

  sys_rw_lock_writer_lock(&type_rw_lock);

  if (*private_offset >= 0) {
    sys_return_if_fail (*private_offset <= 0xffff);

    sys_assert(node->data.instance.private_size <= 0xffff);
    node->data.instance.private_size = pnode 
      ? pnode->data.instance.private_size + *private_offset
      : *private_offset;
    *private_offset = -(SysInt)node->data.instance.private_size;
  }

  sys_rw_lock_writer_unlock(&type_rw_lock);
}

static void iface_entry_add(IFaceEntry *entry) {

  g_iface_entries = sys_slist_prepend(g_iface_entries, entry);
}

static void iface_entry_free(IFaceEntry *entry) {
  sys_return_if_fail(entry != NULL);

  sys_free(entry->iface_ptr);
  sys_free(entry);
}

void sys_type_class_free(SysTypeClass *cls) {
  SysTypeNode *node;
  SysType type;

  sys_return_if_fail(cls != NULL);

  type = sys_type_from_class(cls);
  node = sys_type_node(type);

  if (node->data.instance.class_finalize) {
    node->data.instance.class_finalize(node->data.instance.class_ptr);
  }

  if (cls != NULL) {
    if(cls->n_ifaces > 0) {

      sys_clear_pointer(&cls->ifaces, sys_free);
    }

    sys_free(node->data.instance.class_ptr);
  }
}

static int a = 0;

void sys_type_class_unref(SysTypeClass *cls) {
  if(++a == 1)
  {
    puts("debugger put here");
  }
  SysTypeNode *node = sys_type_node(cls->type);
  sys_assert(node != NULL);

  if (!sys_ref_count_dec(node)) {
    return;
  }

  sys_type_class_free(cls);
}

static SysTypeNode *sys_type_node_ref(SysTypeNode *node) {
  SysTypeNode *pnode;
  SysTypeClass *cls, *pcls;

  sys_ref_count_inc(node);

  switch (node->node_type) {
    case SYS_NODE_FUNDAMENTAL:
      break;
    case SYS_NODE_ABSTRACT_CLASS:
      break;
    case SYS_NODE_CLASS:
    case SYS_NODE_BASE_CLASS:
      cls = node->data.instance.class_ptr;

      if(cls == NULL) {
        cls = (SysTypeClass *)sys_malloc(node->data.instance.class_size);
        cls->type = NODE_TYPE(node);
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

        sys_assert(node->data.instance.private_size >= 0 
            && "check class inherit and GET_CLASS OR CLASS use wrong?.");
      }
      break;
    case SYS_NODE_INTERFACE:
      break;
    default:
      sys_abort_N("Not correct declare type for node: %s", node->name);
      break;
  }

  return node;
}

static SysTypeClass *sys_type_node_class_ref(SysTypeNode *node) {
  sys_return_val_if_fail(node != NULL, NULL);

  node = sys_type_node_ref(node);

  return node->data.instance.class_ptr;
}

SysTypeClass *sys_type_class_ref(SysType type) {
  sys_return_val_if_fail(type != 0, NULL);
  SysTypeNode *node = sys_type_node(type);

  return sys_type_node_class_ref(node);
}

SysBool sys_type_instance_create(
    SysTypeInstance *instance,
    SysTypeNode *node) {
  sys_return_val_if_fail(instance != NULL, false);

  SysTypeNode *pnode;
  SysTypeClass *cls = sys_type_node_class_ref(node);
  if(cls == NULL) { return false; }
  instance->type_class = cls;
  instance->type = NODE_TYPE(node);

  for (SysInt i = node->n_supers; i > 0; i--) {
    SysType p = node->supers[i];
    pnode = sys_type_node(p);

    if (pnode->node_type & SYS_NODE_CLASS) {
      pnode->data.instance.instance_init(instance);
    }
  }

  node->data.instance.instance_init(instance);

  return true;
}

static SysBool instance_destroy(SysTypeInstance *instance, SysTypeClass *cls) {
  sys_return_val_if_fail(instance != NULL, false);

  sys_type_class_unref(cls);

  return true;
}

SysBool sys_type_instance_get_size(SysType type, SysSize *size, SysSize *priv_size) {
  sys_return_val_if_fail(type != 0, 0);
  SysTypeNode *node = sys_type_node(type);

  *priv_size = node->data.instance.private_size;
  *size = node->data.instance.instance_size;

  return true;
}

SysTypeInstance *sys_type_instance_new(SysTypeNode *node, SysSize count) {
  sys_return_val_if_fail(node != NULL, NULL);

  SysTypeInstance *instance;
  SysPointer mp;

  SysInt priv_psize = 0;
  priv_psize = node->data.instance.private_size;

  mp = sys_malloc((priv_psize + node->data.instance.instance_size) * count);
  instance = (SysTypeInstance *)((SysChar *)mp + priv_psize);

  return instance;
}

void sys_type_instance_free(SysTypeInstance *instance) {
  SysTypeNode *node;
  SysChar *real_ptr;
  SysTypeClass *cls;
  SysType type;

  sys_return_if_fail(instance != NULL);

  cls = sys_instance_get_class(instance, SysTypeClass);
  type = sys_type_from_class(cls);
  node = sys_type_node(type);

  if(!instance_destroy(instance, cls)) {
    return;
  }

  real_ptr = ((SysChar*)instance) - node->data.instance.private_size;

  sys_free(real_ptr);
}

const SysChar *sys_type_node_name(SysTypeNode *node) {
  if(node == NULL) { return NULL; }

  return node->name;
}

SysType sys_type_get_by_name(const SysChar *name) {
  sys_assert(ht != NULL && "sys types should be initiated before use.");

  return (SysType)sys_hash_table_lookup(ht, (const SysPointer)name);
}

SysTypeClass *_sys_type_parent_class(SysType type) {
  SysTypeNode *node, *pnode;

  node = sys_type_node(type);
  if(node->n_supers <= 0) {
    return NULL;
  }

  pnode = sys_type_node(NODE_PARENT(node));

  return (SysTypeClass *)pnode->data.instance.class_ptr;
}

void sys_type_teardown(void) {
  sys_hash_table_unref(ht);
  ht = NULL;
  sys_mutex_clear(&param_lock);
  sys_rw_lock_clear(&type_rw_lock);
  sys_rec_mutex_clear(&class_init_rec_mutex);

  sys_slist_free_full(g_iface_entries, (SysDestroyFunc)iface_entry_free);
  g_iface_entries = NULL;
}

void sys_type_setup(void) {
  sys_assert(ht == NULL && "type should be initiated once.");

  sys_rw_lock_init(&type_rw_lock);
  sys_rec_mutex_init(&class_init_rec_mutex);
  sys_mutex_init(&param_lock);

  ht = sys_hash_table_new_full(sys_str_hash,
      (SysEqualFunc)sys_str_equal,
      NULL,
      (SysDestroyFunc)sys_type_node_free);

  _sys_fundanmental_node_init_type();
  _sys_char_node_init_type();
  _sys_double_node_init_type();
  _sys_int_node_init_type();
  _sys_bool_node_init_type();
  _sys_pointer_node_init_type();
  _sys_interface_init_type();
  _sys_object_init_type();
}

SysBool sys_type_node_is(SysTypeNode *node, SYS_NODE_ENUM tp) {
  sys_return_val_if_fail(node != NULL, false);

  return node->node_type & tp;
}

SysType sys_node_get_type(SysTypeNode *node) {
  sys_return_val_if_fail(node != NULL, 0);

  return NODE_TYPE(node);
}

SysTypeNode* sys_type_node(SysType utype) {
  if(utype == 0) { return NULL; }

  SysTypeNode* node;
  if (utype > SYS_TYPE_FUNDAMENTAL_MAX)
    node = (SysTypeNode*)(utype & ~TYPE_ID_MASK);
  else
    node = static_fundamental_type_nodes[utype >> SYS_TYPE_FUNDAMENTAL_SHIFT];

  sys_return_val_if_fail(node != NULL, NULL);
  sys_return_val_if_fail(node->name != NULL, NULL);
  sys_return_val_if_fail(sys_ref_count_check(node, MAX_REF_NODE), NULL);

  return node;
}

SysBool sys_type_node_is_a(SysTypeNode *child, SysTypeNode *parent) {

  return NODE_IS_ANCESTOR(parent, child);
}

SysBool sys_type_is_a(SysType child, SysType parent) {
  sys_return_val_if_fail(child != 0, false);
  sys_return_val_if_fail(parent != 0, false);

  SysTypeNode *ancestor = sys_type_node(parent);
  SysTypeNode *node = sys_type_node(child);

  return NODE_IS_ANCESTOR(ancestor, node);
}

/* interface */

static IFaceEntry* instance_get_interface(IFaceEntry **ifaces,
    SysUInt n_ifaces, 
    SysType iface_type) {
  for (SysUInt j = 0; j < n_ifaces; j++) {
    IFaceEntry* iface = ifaces[j];

    if (iface->iface_type == iface_type) {
      return iface;
    }
  }

  return NULL;
}

static SysTypeInterface* sys_type_class_get_iface(SysTypeNode *node, SysType iface_type) {
  IFaceEntry* entry;
  SysTypeClass *cls;

  for (SysUInt i = 0; i < node->n_supers; i++) {
    cls = sys_type_class_ref(node->supers[i]);
    entry = instance_get_interface(cls->ifaces, cls->n_ifaces, iface_type);

    if (entry != NULL) {

      return entry->iface_ptr;
    }
  }

  return NULL;
}

SysTypeInterface* _sys_type_get_interface(SysType type, SysType iface_type) {
  SysTypeNode *iface_node = sys_type_node(iface_type);
  sys_return_val_if_fail(NODE_IS_IFACE(iface_node), NULL);
  SysTypeNode *node = sys_type_node(type);

  return sys_type_class_get_iface(node, iface_type);
}

/**
 * simple implementation
 */
void sys_type_imp_interface(SysType instance_type,
    SysType iface_type, 
    const SysInterfaceInfo *info) {
  sys_return_if_fail(instance_type > 0);
  sys_return_if_fail(iface_type > 0);
  sys_return_if_fail(info != NULL);

  IFaceEntry* entry;
  IFaceEntry** nmem;
  SysTypeNode *nnode;
  SysTypeNode *iface_node;
  SysTypeNode *node;
  SysTypeInterface *iface_ptr;
  SysTypeClass *cls, *pcls;

  sys_rec_mutex_lock(&class_init_rec_mutex);
  cls = sys_type_class_ref(instance_type);

  sys_rw_lock_writer_lock (&type_rw_lock);
  node = sys_type_node (instance_type);
  if (node == NULL || !(node->node_type & SYS_NODE_CLASS)) {
    sys_warning_N("instance type node not found when implement interface: %p",
        instance_type);
  }
  pcls = sys_type_class_ref(NODE_PARENT(node));

  iface_node = sys_type_node (iface_type);
  if (iface_node == NULL) {
    sys_abort_N("interface node not found when implements interface: %p",
        iface_type);
  }

  if (!(iface_node->node_type & SYS_NODE_INTERFACE)) {
    sys_abort_N("node is not interface type: %s,%s,%p",
        node->name, 
        iface_node->name, 
        iface_type);
  }

  if(!info->interface_init) {
    sys_abort_N("interface init function required: %s,%s",
        node->name, 
        iface_node->name);
  }

  iface_ptr = sys_malloc0(iface_node->data.iface.vtable_size);
  iface_ptr->type = iface_type;

  for(SysUInt i = 1; i < iface_node->n_supers; i++) {
    nnode = sys_type_node(iface_node->supers[i]);

    sys_assert((nnode->node_type & SYS_NODE_INTERFACE) 
        && "iface inherit must be interface");

    /* interface default init in parent. */
    nnode->data.iface.vtable_init(iface_ptr);
  }
  iface_node->data.iface.vtable_init(iface_ptr);

  entry = sys_malloc0(sizeof(IFaceEntry));
  entry->instance_type = instance_type;
  entry->iface_type = iface_type;
  entry->iface_ptr = iface_ptr;

  nmem = sys_malloc0(sizeof(IFaceEntry *) * (cls->n_ifaces + 1));
  if (cls->n_ifaces > 0) {
    /* ifaces = Inew + Iself */
    sys_memcpy(nmem + 1,
        sizeof(IFaceEntry *) * cls->n_ifaces, 
        cls->ifaces, 
        sizeof(IFaceEntry *) * cls->n_ifaces);

    sys_clear_pointer(&cls->ifaces, sys_free);
  }

  cls->ifaces = nmem;
  cls->ifaces[0] = entry;
  cls->n_ifaces++;
  iface_entry_add(entry);

  /* implements interface */
  info->interface_init(iface_ptr);

  sys_type_class_unref(cls);
  sys_type_class_unref(pcls);

  sys_rec_mutex_unlock(&class_init_rec_mutex);
  sys_rw_lock_writer_unlock(&type_rw_lock);
}
