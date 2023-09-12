#include <System/Platform/Common/SysSocketPrivate.h>

#if USE_OPENSSL
SysSocket *sys_socket_new_ssl(int domain, int type, int protocol, SysBool noblocking, SSL_CTX *ssl_ctx) {
  sys_return_val_if_fail(ssl_ctx != NULL, NULL);

  SysSocket *s;
  SSL *ssl;
  
  s = sys_socket_new_I(domain, type, protocol, noblocking);
  sys_return_val_if_fail(s != NULL, NULL);

  ssl = SSL_new(ssl_ctx);
  sys_return_val_if_fail(ssl != NULL, NULL);

  s->ssl = ssl;
  SSL_set_fd(s->ssl, (int)s->fd);

  return s;
}

  SSL* sys_socket_get_ssl(SysSocket* s) {
  return s->ssl;
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

