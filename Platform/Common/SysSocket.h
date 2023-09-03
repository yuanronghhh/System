#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysSocket SysSocket;

struct _SysSocket {

  /* < private > */
#if SYS_OS_WIN32
  SOCKET fd;
#elif SYS_OS_UNIX
  SysInt fd;
#endif
};

#define SYS_FD_SET(socket, fdset) FD_SET((socket)->fd, fdset)
#define SYS_FD_ISSET(socket, fdset) FD_ISSET((socket)->fd, fdset)
#define SYS_FD_ZERO(fdset) FD_ZERO(fdset)

SYS_API SysSocket *sys_socket_new(SysInt fd);
SYS_API SysSocket *sys_socket(int domain, int type, int protocol);
SYS_API void sys_socket_free(SysSocket *s);
SYS_API int sys_socket_setopt(SysSocket *s, int level, int optname, const void *optval, socklen_t optlen);
SYS_API int sys_listen(SysSocket *s, int backlog);
SYS_API SysSocket* sys_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen);
SYS_API int sys_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
SYS_API int sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
SYS_API void sys_freeaddrinfo(struct addrinfo *res);
SYS_API int sys_connect(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
SYS_API SysInt sys_socket_get_fd(SysSocket *s);
SYS_API SysSSize sys_socket_recv(SysSocket *s, void *buf, size_t len, int flags);
SYS_API SysSSize sys_socket_send(SysSocket *s, const void *buf, size_t len, int flags);

SYS_END_DECLS

#endif
