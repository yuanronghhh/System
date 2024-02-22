#include <System/Platform/Common/SysSocketPrivate.h>


SYS_DEFINE_TYPE(SysSocket, sys_socket, SYS_TYPE_OBJECT);

static int ssl_verify_callback(int ok, X509_STORE_CTX* x509_store) {
#if SYS_DEBUG
  char* subject, * issuer;
  int                err, depth;
  X509* cert;
  X509_NAME* sname, * iname;

  // TODO: SSL_set_ex_data not called
  SysPointer _ = X509_STORE_CTX_get_ex_data(x509_store,
    SSL_get_ex_data_X509_STORE_CTX_idx());

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
#endif

  return 1;
}

SysSocket *sys_socket_new_ssl(int domain, int type, int protocol, SysBool noblocking, SSL_CTX *ssl_ctx) {
  sys_return_val_if_fail(ssl_ctx != NULL, NULL);

  SysSocket *s;
  SSL *ssl;

  s = sys_socket_new_I(domain, type, protocol, noblocking);

  sys_return_val_if_fail(s != NULL, NULL);

  ssl = SSL_new(ssl_ctx);
  if(ssl == NULL) {
    return NULL;
  }
  sys_socket_set_ssl(s, ssl);

  if (SSL_get_verify_result(ssl) != X509_V_OK) {
    goto fail;
  }

  return s;

fail:
  sys_warning_N("%s", sys_socket_error());
  if(ssl != NULL) {
    SSL_free(ssl);
  }

  sys_object_unref(s);
  return NULL;
}

SSL* sys_socket_get_ssl(SysSocket* s) {
  sys_return_val_if_fail(s != NULL, NULL);

  return s->ssl;
}

void sys_socket_set_ssl(SysSocket* s, SSL* ssl) {
  sys_return_if_fail(s != NULL);

  s->ssl = ssl;
  SSL_set_fd(ssl, (int)s->fd);
}

SysSocket *sys_socket_new_I(int domain, int type, int protocol, SysBool noblocking) {
  SYS_LEAK_IGNORE_BEGIN;
  SysSocket *ns = sys_socket_real_new_I(domain, type, protocol, noblocking);
  SYS_LEAK_IGNORE_END;

  if (ns == NULL) {
    sys_warning_N("socket: %s", sys_socket_error());

    return NULL;
  }

  return ns;
}

SysSocket *sys_socket_new_fd(SOCKET fd) {
  SysSocket *s = sys_socket_new();

  s->fd = fd;
  s->noblocking = false;

  return s;
}

SysInt sys_socket_set_blocking(SysSocket *s, SysBool bvalue) {
  SysULong ul = bvalue;

  return sys_socket_ioctl(s, FIONBIO, &ul);
}

int sys_socket_send(SysSocket *s, const void *buf, size_t len, int flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return sys_socket_real_send(s, buf, len, flags);
}

const char *sys_socket_error(void) {

  return sys_socket_strerror(sys_socket_errno());
}

int sys_socket_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);
  int r, nr;

  r = sys_socket_real_connect(s, addr, addrlen);
  if (r < 0) {

    sys_warning_N("connect: %s", sys_socket_error());
  }

  if (s->ssl) {
    nr = SSL_connect(s->ssl);

    if (nr < 0) {
      sys_warning_N("ssl accept: %s", sys_ssl_error());
      return -1;
    }
  }

  return r;
}

SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SysSocket* cs;
  SSL *ssl;
  SysInt r;

  cs = sys_socket_real_accept(s, addr, addrlen);
  if(cs == NULL) {

    sys_warning_N("accept: %s", sys_socket_error());
    return NULL;
  }

  if (s->ssl) {
      r = SSL_accept(s->ssl);
      if (r <= 0) {
          sys_warning_N("ssl accept: %s", sys_ssl_error());
          sys_object_unref(cs);
          return NULL;
      }

      ssl = SSL_new(sys_ssl_ctx_get_client());
      sys_socket_set_ssl(cs, ssl);
      cs->ssl = ssl;
  }

  return cs;
}

int sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = bind(s->fd, addr, addrlen);
  if (r < 0) {

    sys_warning_N("bind: %s", sys_socket_error());
  }

  return r;
}

int sys_socket_recv(SysSocket *s, void *buf, size_t len, int flags) {
  sys_return_val_if_fail(s != NULL, -1);
  int r;

  if (s->ssl) {
    r = SSL_read(s->ssl, buf, (int)len);
    if (r < 0) {

      sys_warning_N("recv: %s", sys_ssl_error());
    }

  } else {

    r = sys_socket_real_recv(s, buf, (int)len, flags);
    if (r < 0) {

      sys_warning_N("recv: %s", sys_socket_error());
    }
  }

  return r;
}

SysInt sys_socket_ioctl(SysSocket *s, long cmd, u_long * argp) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = sys_socket_real_ioctl(s, cmd, argp);
  if (r < 0) {

    sys_warning_N("ioctlsocket: %s", sys_socket_error());
  }
  return r;
}

SOCKET sys_socket_get_fd(SysSocket *s) {
  sys_return_val_if_fail(s != NULL, -1);

  return s->fd;
}

/* object api */
SysSocket* sys_socket_new(void) {
  return sys_object_new(SYS_TYPE_SOCKET, NULL);
}

static void sys_socket_dispose(SysObject* o) {
  SysSocket *self = SYS_SOCKET(o);

  if (self->ssl) {
    SSL_shutdown(self->ssl);
    SSL_free(self->ssl);
  }

  sys_socket_real_close(self);

  SYS_OBJECT_CLASS(sys_socket_parent_class)->dispose(o);
}

static void sys_socket_class_init(SysSocketClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_socket_dispose;
}

void sys_socket_init(SysSocket* self) {
}

