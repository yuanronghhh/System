#ifndef __SYS_SOCKET_CONNECTION__
#define __SYS_SOCKET_CONNECTION__

#include <System/Platform/Common/SysSocketAddr.h>

SYS_BEGIN_DECLS

#define SOCKET_TYPE_CONNECTION (sys_socket_connection_get_type())
#define SYS_SOCKET_CONNECTION(o) ((SysSocketConnection* )sys_object_cast_check(o, SOCKET_TYPE_CONNECTION))
#define SYS_SOCKET_CONNECTION_CLASS(o) ((SysSocketConnectionClass *)sys_class_cast_check(o, SOCKET_TYPE_CONNECTION))
#define SYS_SOCKET_CONNECTION_GET_CLASS(o) sys_instance_get_class(o, SysSocketConnectionClass)

struct _SysSocketConnection {
  SysObject parent;

  /* < private > */
  SysSocket *socket;
  SysSocketAddrIn laddr;
  SysSocketAddrIn raddr;
};

struct _SysSocketConnectionClass {
  SysObjectClass parent;

};

SYS_API SysType sys_socket_connection_get_type(void);
SYS_API SysSocketConnection *sys_socket_connection_new(void);
SYS_API SysSocketConnection* sys_socket_connection_new_I(
    SysSocket *socket,
    SysSocketAddrIn *laddr,
    SysSocketAddrIn *raddr);

SYS_API SysBool sys_socket_connection_listen(SysSocketConnection *self);
SYS_API SysSSize sys_socket_connection_connect(SysSocketConnection *self);
SYS_API SysSocket *sys_socket_connection_get_socket(SysSocketConnection *self);
SYS_API SysSocketAddrIn* sys_socket_connection_get_raddr(SysSocketConnection* self);
SYS_API SysSocketAddrIn* sys_socket_connection_get_laddr(SysSocketConnection* self);
SYS_API SysSSize sys_socket_connection_pipe(SysSocketConnection* cconn, SysSocketConnection *rconn);
SYS_API SysBool sys_socket_connection_is_connected(SysSocketConnection* self);

SYS_END_DECLS

#endif

