#include <System/Utils/SysSsl.h>
#include <System/Platform/Common/SysSocket.h>

static SysInt ssl_verify_callback(SysInt ok, X509_STORE_CTX* x509_store) {
  SysChar* subject, * issuer;
  SysInt                err, depth;
  X509* cert;
  X509_NAME* sname, * iname;

  // TODO: SSL_set_ex_data not called
  SysPointer _ = X509_STORE_CTX_get_ex_data(x509_store,
    SSL_get_ex_data_X509_STORE_CTX_idx());
  UNUSED(_);

  cert = X509_STORE_CTX_get_current_cert(x509_store);
  err = X509_STORE_CTX_get_error(x509_store);
  depth = X509_STORE_CTX_get_error_depth(x509_store);

  sname = X509_get_subject_name(cert);

  if (sname) {
    subject = X509_NAME_oneline(sname, NULL, 0);
    if (subject == NULL) {
      sys_error_N("%s", "X509_NAME_oneline() failed");
    }

  } else {
    subject = NULL;
  }

  iname = X509_get_issuer_name(cert);

  if (iname) {
    issuer = X509_NAME_oneline(iname, NULL, 0);
    if (issuer == NULL) {
      sys_error_N("%s", "X509_NAME_oneline() failed");
    }

  } else {
    issuer = NULL;
  }

  sys_debug_N("verify:%d, error:%d, depth:%d, "
    "subject:\"%s\", issuer:\"%s\"",
    ok, err, depth,
    subject ? subject : "(none)",
    issuer ? issuer : "(none)");

  if (subject) {

    OPENSSL_free(subject);
  }

  if (issuer) {
    OPENSSL_free(issuer);
  }

  return 1;
}

