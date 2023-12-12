#ifndef __SYS_SOCKET_PRIVATE_H__
#define __SYS_SOCKET_PRIVATE_H__

#include <System/Platform/Common/SysSocket.h>

SYS_BEGIN_DECLS

struct _SysSocket {
  /* < private > */
#if SYS_OS_WIN32
  SOCKET fd;
#elif SYS_OS_UNIX
  SysInt fd;
#endif

  SysBool noblocking;

#if USE_OPENSSL
  SSL *ssl;
  // BIO *bio;

  /* maybe NULL */
  // BIO *ssl_bio;
  // SysSocket *ssl_s;
#endif
};

SysSocket *sys_socket_real_new_I(int domain, int type, int protocol, SysBool noblocking);
int sys_socket_real_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen);
SysSocket* sys_socket_real_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen);
int sys_socket_real_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
int sys_socket_real_recv(SysSocket *s, void *buf, size_t len, int flags);
void sys_socket_real_close(SysSocket *s);
SysInt sys_socket_real_ioctl(SysSocket *s, long cmd, u_long * argp);
int sys_socket_real_send(SysSocket *s, const void *buf, size_t len, int flags);

SYS_END_DECLS

#endif
