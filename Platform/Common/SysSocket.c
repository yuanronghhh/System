#include "SysSocket.h"
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
  ERR_print_errors_fp(stderr);
  if(ssl != NULL) {
    SSL_free(ssl);
  }

  sys_socket_free(s);
  return NULL;
}

SSL* sys_socket_get_ssl(SysSocket* s) {
  return s->ssl;
}

void sys_socket_set_ssl(SysSocket* s, SSL* ssl) {
  s->ssl = ssl;
}
#endif

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

    sys_debug_N("connect: %s", sys_socket_error());
  }

#if USE_OPENSSL
  // if (SSL_connect(s->ssl) <= 0) {
  //   sys_ssl_error();
  //   return -1;
  // }
#endif

  return r;
}
