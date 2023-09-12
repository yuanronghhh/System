#include <System/Utils/SysOpenSSL.h>


void sys_ssl_setup(void) {
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

}

void sys_ssl_teardown(void) {
}
