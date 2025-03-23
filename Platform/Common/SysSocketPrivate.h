#ifndef __SYS_SOCKET_PRIVATE_H__
#define __SYS_SOCKET_PRIVATE_H__

#include <System/Platform/Common/SysSocket.h>

SYS_BEGIN_DECLS

SysSocket *sys_socket_real_new_I(SysInt domain, SysInt type, SysInt protocol);
SysBool sys_socket_real_set_no_blocking(SysSocket *s, SysBool bvalue);
SysInt sys_socket_real_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen);
SysSocket* sys_socket_real_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen);
SysInt sys_socket_real_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen);
SysInt sys_socket_real_recv(SysSocket *s, void *buf, size_t len, SysInt flags);
SysInt sys_socket_real_read(SysSocket *s, void *buf, size_t len);
void sys_socket_real_shutdown(SysSocket *s, int flags);
void sys_socket_real_close(SysSocket *s);
SysInt sys_socket_real_ioctl(SysSocket *s, long cmd, u_long * argp);
SysInt sys_socket_real_send(SysSocket *s, const void *buf, size_t len, SysInt flags);

SYS_END_DECLS

#endif
