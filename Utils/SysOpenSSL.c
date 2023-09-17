#include <System/Utils/SysOpenSSL.h>
#include <System/Utils/SysError.h>

static SysInt server_conf_index;
static SysInt certificate_index;

void sys_ssl_setup(void) {
  int n;
  STACK_OF(SSL_COMP) *ssl_comp_methods;

#if OPENSSL_VERSION_NUMBER >= 0x10100003L
    if (OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL) == 0) {
        sys_error_N("%s", "OPENSSL_init_ssl() failed");
        return;
    }

    ERR_clear_error();
#else

  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
#endif

  ssl_comp_methods = SSL_COMP_get_compression_methods();
  n = sk_SSL_COMP_num(ssl_comp_methods);

  while (n--) {

    (void)sk_SSL_COMP_pop(ssl_comp_methods);
  }
}

void sys_ssl_teardown(void) {
#if OPENSSL_VERSION_NUMBER < 0x10100003L

  EVP_cleanup();
#ifndef OPENSSL_NO_ENGINE
  ENGINE_cleanup();
#endif
#endif
}

static int ssl_verify_callback(int ok, X509_STORE_CTX *x509_store)
{
  char              *subject, *issuer;
  int                err, depth;
  X509              *cert;
  X509_NAME         *sname, *iname;

  cert = X509_STORE_CTX_get_current_cert(x509_store);
  err = X509_STORE_CTX_get_error(x509_store);
  depth = X509_STORE_CTX_get_error_depth(x509_store);

  sname = X509_get_subject_name(cert);
  subject = sname ? X509_NAME_oneline(sname, NULL, 0) : "(none)";

  iname = X509_get_issuer_name(cert);
  issuer = iname ? X509_NAME_oneline(iname, NULL, 0) : "(none)";

  sys_debug_N("verify:%d, error:%d, depth:%d, "
    "subject:\"%s\", issuer:\"%s\"",
    ok, err, depth, subject, issuer);

  if (sname) {
    OPENSSL_free(subject);
  }

  if (iname) {
    OPENSSL_free(issuer);
  }

  return 1;
}

SSL_CTX *sys_ssl_ctx_new(SysPointer data) {
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());

  if (SSL_CTX_set_ex_data(ctx, server_conf_index, data) == 0) {
    goto fail;
  }

  if (SSL_CTX_set_ex_data(ctx, certificate_index, NULL) == 0) {
    goto fail;
  }

  SSL_CTX_set_read_ahead(ctx, 1);

  return ctx;

fail:
  sys_ssl_error();
  SSL_CTX_free(ctx);
  return NULL;
}

SSL_CTX* sys_ssl_create_server_ctx(const SysChar *signed_file, const SysChar *priv_file) {
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_server_method());

  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, ssl_verify_callback);

  if (SSL_CTX_use_PrivateKey_file(ctx, priv_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_use_certificate_file(ctx, signed_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_check_private_key(ctx) <= 0) {
    goto fail;
  }

  return ctx;
fail:
  sys_ssl_error();
  SSL_CTX_free(ctx);

  ERR_clear_error();
  return NULL;
}

SSL_CTX* sys_ssl_create_client_ctx(const SysChar* ca_file, const SysChar* priv_file) {
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());

  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, ssl_verify_callback);

  if (SSL_CTX_load_verify_locations(ctx, ca_file, NULL) <= 0) {
    goto fail;
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, priv_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_use_certificate_file(ctx, ca_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_check_private_key(ctx) <= 0) {
    goto fail;
  }

  return ctx;
fail:
  sys_ssl_error();
  SSL_CTX_free(ctx);

  ERR_clear_error();
  return NULL;
}

void sys_ssl_error(void) {
  unsigned long ssl_code = ERR_get_error();
  sys_warning_N("ssl: %d,%s", ssl_code, ERR_error_string(ssl_code, NULL));
}
