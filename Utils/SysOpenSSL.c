#include "SysOpenSSL.h"
#include <System/Utils/SysOpenSSL.h>


void sys_ssl_setup(void) {
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
}

void sys_ssl_teardown(void) {
}

SSL_CTX* sys_ssl_create_server_ctx(const SysChar *signed_key, const SysChar *priv_key) {
  SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_server_method());

  if (SSL_CTX_use_certificate_file(ssl_ctx, signed_key, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_use_PrivateKey_file(ssl_ctx, priv_key, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_check_private_key(ssl_ctx) <= 0) {
    goto fail;
  }

  return ssl_ctx;

fail:
  SSL_CTX_free(ssl_ctx);
  return NULL;
}

SSL_CTX* sys_ssl_create_client_ctx(const SysChar* ca_key, const SysChar* priv_key) {
  SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_client_method());

  if (SSL_CTX_load_verify_locations(ssl_ctx, ca_key, NULL) <= 0) {
    goto fail;
  }

  if (SSL_CTX_use_PrivateKey_file(ssl_ctx, priv_key, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_check_private_key(ssl_ctx) <= 0) {
    goto fail;
  }

  if (SSL_get_verify_result(ssl_ctx) != X509_V_OK) {
    goto fail;
  }

  return ssl_ctx;
fail:
  ERR_print_errors_fp(stderr);
  SSL_CTX_free(ssl_ctx);
  return NULL;
}