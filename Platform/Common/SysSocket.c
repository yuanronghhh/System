#include <System/Platform/Common/SysSocketPrivate.h>

#if USE_OPENSSL
SSL* sys_socket_get_ssl(SysSocket* s) {
  return s->ssl;
}

SysSocket* sys_socket_new_ssl(SOCKET fd, SysBool noblocking, SSL *ssl) {
  SysSocket *s = sys_socket_new_I(fd, noblocking);
  s->ssl = ssl;
  SSL_set_fd(s->ssl, (int)s->fd);

  return s;
}
#endif

SysSocket *sys_socket_new_I(SOCKET fd, SysBool noblocking) {
  SysSocket *s = sys_new0_N(SysSocket, 1);

  s->fd = fd;
  s->noblocking = noblocking;

  sys_socket_set_noblock(s, noblocking);

  return s;
}

SysInt sys_socket_set_noblock(SysSocket *s, SysBool bvalue) {
  SysULong ul = bvalue;

  return sys_socket_ioctl(s, FIONBIO, &ul);
}

