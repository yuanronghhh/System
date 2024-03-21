#include <System/TestSuite/BHeap.h>

#define POINTER_TO_DOUBLE(o) ((SysDouble)(SysUInt64)o)

static SysDouble score_func(SysPointer node) {

  return POINTER_TO_DOUBLE(node);
}

static void node_destroy_cb(SysPointer node) {
}

static void test_bheap_basic(void) {
  SysPointer v;
  SysBHeap* hp = sys_bheap_new(score_func, node_destroy_cb);
  int sz = sizeof(double);

  for (SysUInt64 i = 3; i < 100; i++) {
    v = (SysPointer)(SysUInt64)(i % 11);
    sys_debug_N("%ld", v);

    sys_bheap_push(hp, INT_TO_POINTER(v));
  }
  // sys_bheap_remove(hp, INT_TO_POINTER(3));

  while ((v = sys_bheap_pop(hp)) != NULL) {

    sys_debug_N("%ld", v);
  }

  sys_bheap_free(hp, true);
}

void test_bheap_init(int argc, SysChar* argv[]) {
  UNITY_BEGIN();
  {
    RUN_TEST(test_bheap_basic);
  }
  UNITY_END();
}
