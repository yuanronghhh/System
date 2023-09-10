#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysSocket SysSocket;

#if SYS_OS_UNIX
#define SOCKET SysInt
#endif

SYS_API SysSocket *sys_socket_new(SOCKET fd);
SYS_API SysSocket *sys_socket(int domain, int type, int protocol);
SYS_API void sys_socket_free(SysSocket *s);
SYS_API int sys_socket_setopt(SysSocket *s, int level, int optname, const void *optval, socklen_t optlen);
SYS_API int sys_socket_listen(SysSocket *s, int backlog);
SYS_API SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen);
SYS_API int sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
SYS_API int sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
SYS_API void sys_freeaddrinfo(struct addrinfo *res);
SYS_API int sys_socket_connect(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
SYS_API SOCKET sys_socket_get_fd(SysSocket *s);
SYS_API int sys_socket_recv(SysSocket *s, void *buf, size_t len, int flags);
SYS_API int sys_socket_send(SysSocket *s, const void *buf, size_t len, int flags);
SYS_API int sys_socket_ioctl(SysSocket * s, long cmd, u_long * argp);
SYS_API const char* sys_socket_strerror(int err);
SYS_API int sys_socket_errno(void);

SYS_END_DECLS

#endif
