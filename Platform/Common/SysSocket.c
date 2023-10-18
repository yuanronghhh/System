#include <System/Platform/Common/SysSocketPrivate.h>

#if USE_OPENSSL
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

  s->ssl = ssl;
  SSL_set_fd(s->ssl, (int)s->fd);

  if (SSL_get_verify_result(ssl) != X509_V_OK) {
    goto fail;
  }

  return s;

fail:
  sys_warning_N("%s", sys_socket_error());
  if(ssl != NULL) {
    SSL_free(ssl);
  }

  sys_socket_close(s);
  return NULL;
}

  SSL* sys_socket_get_ssl(SysSocket* s) {
  return s->ssl;
}
#endif

SysSocket *sys_socket_new_I(int domain, int type, int protocol, SysBool noblocking) {
  SysSocket *ns = sys_socket_real_new_I(domain, type, protocol, noblocking);
  if (ns == NULL) {
    sys_warning_N("socket: %s", sys_socket_error());

    return NULL;
  }

  return ns;
}

void sys_socket_close(SysSocket *s) {
  sys_return_if_fail(s != NULL);

#if USE_OPENSSL
  sys_assert(s->ssl != NULL);

  SSL_shutdown(s->ssl);
  SSL_free(s->ssl);
#else
  sys_socket_real_close(s);
#endif

  sys_free_N(s);
}

SysSocket *sys_socket_new_fd(SOCKET fd) {
  SysSocket *s = sys_new0_N(SysSocket, 1);

  s->fd = fd;
  s->noblocking = false;

  return s;
}

SysInt sys_socket_set_blocking(SysSocket *s, SysBool bvalue) {
  SysULong ul = bvalue;

  return sys_socket_ioctl(s, FIONBIO, &ul);
}

const char *sys_socket_error(void) {
  return sys_socket_strerror(sys_socket_errno());
}

int sys_socket_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = sys_socket_real_connect(s, addr, addrlen);
  if (r < 0) {

    sys_warning_N("connect: %s", sys_socket_error());
  }

#if USE_OPENSSL
  // if (SSL_connect(s->ssl) <= 0) {
  //   sys_warning_N("%s", sys_ssl_error());
  //   return -1;
  // }
  SSL_set_connect_state(s->ssl);
#endif

  return r;
}

SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);

  SysSocket *ns = sys_socket_real_accept(s, addr, addrlen);
  if(ns == NULL) {

    sys_warning_N("accept: %s", sys_socket_error());
    return NULL;
  }

#if USE_OPENSSL
  // if ((r = SSL_accept(s->ssl)) <= 0) {
  //   sys_warning_N("ssl accept: %s", sys_ssl_error());
  //   return NULL;
  // }

  SSL_set_accept_state(ns->ssl);
#endif

  return ns;
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

#if USE_OPENSSL
  r = SSL_read(s->ssl, buf, (int)len);
  if (r < 0) {

    sys_warning_N("recv: %s", sys_ssl_error());
  }
#else

  r = sys_socket_real_recv(s, buf, (int)len, flags);
  if (r < 0) {

    sys_warning_N("recv: %s", sys_socket_error());
  }
#endif

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
