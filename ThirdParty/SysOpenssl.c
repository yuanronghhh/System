#include <System/ThirdParty/SysOpenssl.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Utils/SysError.h>

static SysMutex ssl_ctx_mlock;
static SysInt server_conf_index;
// static SysInt certificate_index;

static SSL_CTX* server_ctx = NULL;
static SSL_CTX* client_ctx = NULL;

void sys_ssl_setup(void) {
  SysInt n;
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

static void sys_ssl_ctx_set_client_option(SSL_CTX* ctx) {
  /* client side options */

#ifdef SSL_OP_MICROSOFT_SESS_ID_BUG
  SSL_CTX_set_options(ctx, SSL_OP_MICROSOFT_SESS_ID_BUG);
#endif

#ifdef SSL_OP_NETSCAPE_CHALLENGE_BUG
  SSL_CTX_set_options(ctx, SSL_OP_NETSCAPE_CHALLENGE_BUG);
#endif
}

static void sys_ssl_ctx_set_server_option(SSL_CTX* ctx) {
  /* server side options */

#ifdef SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG
  SSL_CTX_set_options(ctx, SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG);
#endif

#ifdef SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER
  SSL_CTX_set_options(ctx, SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER);
#endif

#ifdef SSL_OP_MSIE_SSLV2_RSA_PADDING
  /* this option allow a potential SSL 2.0 rollback (CAN-2005-2969) */
  SSL_CTX_set_options(ctx, SSL_OP_MSIE_SSLV2_RSA_PADDING);
#endif

#ifdef SSL_OP_SSLEAY_080_CLIENT_DH_BUG
  SSL_CTX_set_options(ctx, SSL_OP_SSLEAY_080_CLIENT_DH_BUG);
#endif

#ifdef SSL_OP_TLS_D5_BUG
  SSL_CTX_set_options(ctx, SSL_OP_TLS_D5_BUG);
#endif

#ifdef SSL_OP_TLS_BLOCK_PADDING_BUG
  SSL_CTX_set_options(ctx, SSL_OP_TLS_BLOCK_PADDING_BUG);
#endif

#ifdef SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS
  SSL_CTX_set_options(ctx, SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);
#endif

  SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);

#if OPENSSL_VERSION_NUMBER >= 0x009080dfL
  /* only in 0.9.8m+ */
  SSL_CTX_clear_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1);
#endif

#ifdef SSL_CTX_set_min_proto_version
  SSL_CTX_set_min_proto_version(ctx, 0);
  SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
#endif

#ifdef TLS1_3_VERSION
  SSL_CTX_set_min_proto_version(ctx, 0);
  SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
#endif

#ifdef SSL_OP_NO_COMPRESSION
  SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);
#endif

#ifdef SSL_OP_NO_ANTI_REPLAY
  SSL_CTX_set_options(ctx, SSL_OP_NO_ANTI_REPLAY);
#endif

#ifdef SSL_OP_NO_CLIENT_RENEGOTIATION
  SSL_CTX_set_options(ctx, SSL_OP_NO_CLIENT_RENEGOTIATION);
#endif

#ifdef SSL_MODE_RELEASE_BUFFERS
  SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
#endif

#ifdef SSL_MODE_NO_AUTO_CHAIN
  SSL_CTX_set_mode(ctx, SSL_MODE_NO_AUTO_CHAIN);
#endif

  SSL_CTX_set_read_ahead(ctx, 1);
}

SysInt sys_ssl_renegotiate(SSL* ssl) {
  SysInt r = false;
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
  if (SSL_version(ssl) >= TLS1_3_VERSION) {

    r = SSL_key_update(ssl, SSL_KEY_UPDATE_REQUESTED);
  }
  else if (SSL_get_secure_renegotiation_support(ssl) && !(SSL_get_options(ssl) & SSL_OP_NO_RENEGOTIATION)) {

    /* remote and local peers both can rehandshake */
    r = SSL_renegotiate(ssl);
  }
  else {

    sys_warning_N("%s", "Secure renegotiation is not supported");
  }
#else
  r = SSL_renegotiate(ssl);
#endif

  return r;
}

static SysInt ssl_verify_callback(SysInt ok, X509_STORE_CTX* x509_store)
{
  SysChar* subject, * issuer;
  SysInt                err, depth;
  X509* cert;
  X509_NAME* sname, * iname;

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

static SSL_CTX* ssl_ctx_create_server(const SysChar* signed_file, const SysChar* priv_file) {
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_server_method());

  SSL_CTX_set_ex_data(ctx, server_conf_index, NULL);
  sys_ssl_ctx_set_server_option(ctx);
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, ssl_verify_callback);
  SSL_CTX_set_verify_depth(ctx, 0);

  if (!SSL_CTX_use_certificate_chain_file(ctx, signed_file)) {
    goto fail;
  }

  if (SSL_CTX_use_certificate_file(ctx, signed_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, priv_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_check_private_key(ctx) <= 0) {
    goto fail;
  }

  return ctx;
fail:
  sys_warning_N("%s", sys_ssl_error());
  SSL_CTX_free(ctx);

  ERR_clear_error();
  return NULL;
}

static SSL_CTX* ssl_ctx_create_client(const SysChar* ca_file, const SysChar* priv_file) {
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());

  STACK_OF(X509_NAME)* list = NULL;

  sys_ssl_ctx_set_client_option(ctx);

  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, ssl_verify_callback);
  SSL_CTX_set_verify_depth(ctx, 1);

  if (SSL_CTX_load_verify_locations(ctx, ca_file, NULL) <= 0) {
    goto fail;
  }

  list = SSL_load_client_CA_file(ca_file);
  if (list == NULL) {
    sys_warning_N("SSL_load_client_CA_file(\"%s\") failed", ca_file);
    goto fail;
  }
  SSL_CTX_set_client_CA_list(ctx, list);

  if (SSL_CTX_use_certificate_file(ctx, ca_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

#if 0
  if (SSL_CTX_use_PrivateKey_file(ctx, priv_file, SSL_FILETYPE_PEM) <= 0) {
    goto fail;
  }

  if (SSL_CTX_check_private_key(ctx) <= 0) {
    goto fail;
  }
#endif

  return ctx;
fail:
  sys_warning_N("%s", sys_ssl_error());
  SSL_CTX_free(ctx);

  ERR_clear_error();
  return NULL;
}

void sys_ssl_ctx_setup(
  const SysChar *server_crt, 
  const SysChar* server_priv,
  const SysChar* client_ca,
  const SysChar* client_priv) {

  sys_mutex_init(&ssl_ctx_mlock);

  server_ctx = ssl_ctx_create_server(server_crt, server_priv);
  client_ctx = ssl_ctx_create_client(client_ca, client_priv);
}

void sys_ssl_ctx_teardown(void) {
  sys_clear_pointer(&server_ctx, SSL_CTX_free);
  sys_clear_pointer(&client_ctx, SSL_CTX_free);

  sys_mutex_clear(&ssl_ctx_mlock);
}

SSL_CTX * sys_ssl_ctx_get_client(void) {
  SSL_CTX* lctx;

  sys_mutex_lock(&ssl_ctx_mlock);
  lctx = client_ctx;
  sys_mutex_unlock(&ssl_ctx_mlock);

  return lctx;
}

SSL_CTX* sys_ssl_ctx_get_server(void) {
  SSL_CTX* lctx;

  sys_mutex_lock(&ssl_ctx_mlock);
  lctx = server_ctx;
  sys_mutex_unlock(&ssl_ctx_mlock);

  return lctx;
}

const SysChar * sys_ssl_error(void) {
  unsigned long ssl_code = ERR_get_error();
  return ERR_error_string(ssl_code, NULL);
}
