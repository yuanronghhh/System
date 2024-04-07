#include <System/DataTypes/SysQuark.h>
#include <System/Utils/SysString.h>

static SysHashTable *ht;

const SysChar *sys_quark_string(const SysChar *msg) {
  sys_return_val_if_fail(msg, NULL);

  SysChar *nmsg;

  nmsg = (SysChar *)sys_hash_table_lookup(ht, (const SysPointer)msg);
  if(nmsg == NULL) {

    nmsg = sys_strdup(msg);
    sys_hash_table_insert(ht, (SysPointer)nmsg, (SysPointer)nmsg);
  }

  return nmsg;
}

void sys_quark_setup(void) {
  ht = sys_hash_table_new_full(sys_str_hash,
      (SysEqualFunc)sys_str_equal,
      (SysDestroyFunc)sys_free,
      NULL);
}

void sys_quark_teardown(void) {
  sys_hash_table_unref(ht);
}
