#include <System/TestSuite/Socket.h>


static void test_socket_basic(void) {
  SysSocket *s = sys_socket_new_I(AF_INET, SOCK_STREAM, IPPROTO_TCP, false);
  sys_object_unref(s);
}

static void test_ssl_leak(void) {
  sys_ssl_ctx_setup(
    SIGNED_CRT_FILE, PRIVATE_KEY_FILE,
    CA_CRT_FILE, PRIVATE_KEY_FILE);

  sys_ssl_ctx_teardown();
}

void test_socket_init(int argc, SysChar* argv[]) {
  UNITY_BEGIN();
  {
    // RUN_TEST(test_ssl_leak);
    RUN_TEST(test_socket_basic);
  }
  UNITY_END();
}
