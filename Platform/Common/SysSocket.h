#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

#define SYS_TYPE_SOCKET (sys_socket_get_type())
#define SYS_SOCKET(o) ((SysSocket* )sys_object_cast_check(o, SYS_TYPE_SOCKET))
#define SYS_SOCKET_CLASS(o) ((SysSocketClass *)sys_class_cast_check(o, SYS_TYPE_SOCKET))
#define SYS_SOCKET_GET_CLASS(o) sys_instance_get_class(o, SysSocketClass)

typedef struct _SysSocket SysSocket;
typedef struct _SysSocketClass SysSocketClass;

#if SYS_OS_UNIX
#define SOCKET SysInt
#endif

struct _SysSocketClass {
  SysObjectClass parent;
};

struct _SysSocket {
  SysObject parent;
  /* < private > */
#if SYS_OS_WIN32
  SOCKET fd;
#elif SYS_OS_UNIX
  SysInt fd;
#endif

  SysBool noblocking;
  SSL *ssl;
};

SYS_API SysSocket *sys_socket_new_ssl(SysInt domain, SysInt type, SysInt protocol, SysBool noblocking, SSL_CTX * ssl_ctx);
SYS_API SSL* sys_socket_get_ssl(SysSocket* s);
SYS_API void sys_socket_set_ssl(SysSocket* s, SSL* ssl);

SYS_API SysType sys_socket_get_type(void);
SYS_API SysSocket* sys_socket_new(void);

SYS_API SysSocket *sys_socket_new_I(SysInt domain, SysInt type, SysInt protocol, SysBool noblocking);
SYS_API SysSocket *sys_socket_new_fd(SOCKET fd);
SYS_API SysInt sys_socket_set_blocking(SysSocket *s, SysBool bvalue);
SYS_API SysInt sys_socket_setopt(SysSocket *s, SysInt level, SysInt optname, const void *optval, socklen_t optlen);
SYS_API SysInt sys_socket_listen(SysSocket *s, SysInt backlog);
SYS_API SysSocket* sys_socket_accept(SysSocket* s, struct sockaddr* addr, socklen_t* addrlen);
SYS_API SysInt sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
SYS_API SysInt sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
SYS_API void sys_freeaddrinfo(struct addrinfo *res);
SYS_API SysInt sys_socket_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen);
SYS_API SOCKET sys_socket_get_fd(SysSocket *s);
SYS_API SysInt sys_socket_recv(SysSocket *s, void *buf, size_t len, SysInt flags);
SYS_API SysInt sys_socket_send(SysSocket *s, const void *buf, size_t len, SysInt flags);
SYS_API SysInt sys_socket_ioctl(SysSocket * s, long cmd, u_long * argp);
SYS_API const char* sys_socket_strerror(SysInt err);
SYS_API const char * sys_socket_error(void);
SYS_API SysInt sys_socket_errno(void);

SYS_END_DECLS

#endif
