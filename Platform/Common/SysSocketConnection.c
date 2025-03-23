#include <System/Platform/Common/SysSocketConnection.h>
#include <System/Platform/Common/SysSocketAddr.h>
#include <System/Platform/Common/SysSocket.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Utils/SysError.h>

SYS_DEFINE_TYPE(SysSocketConnection, sys_socket_connection, SYS_TYPE_OBJECT);

SysSSize sys_socket_connection_connect(SysSocketConnection *self) {
  sys_return_val_if_fail(self != NULL, -1);
  SysSSize r;

  r = sys_socket_connect(self->socket, &self->raddr, sizeof(struct sockaddr_in));
  if (r < 0) {
    sys_warning_N("connect remote failed: %s:%d",
        sys_socket_addr_get_host(&self->raddr),
        sys_socket_addr_get_port(&self->raddr));

    return r;
  }

  return r;
}

SysBool sys_socket_connection_is_connected(SysSocketConnection* self) {

  return sys_socket_is_connected(self->socket);
}

SysSSize sys_socket_connection_pipe(
    SysSocketConnection* src,
    SysSocketConnection *dst) {
  static SysChar buffer[4096];
  SysSSize r = 0;
  SysSSize c = 0;

  r = sys_socket_recv(src->socket, buffer, sizeof(buffer), 0);
  if (r < 0) {
    return r;
  }

  c = sys_socket_send(dst->socket, buffer, r, 0);

  return c;
}

static void sys_socket_connection_construct(SysSocketConnection* self,
    SysSocketAddrIn *laddr,
    SysSocketAddrIn *raddr,
    SysSocket *socket) {

  sys_return_if_fail(self != NULL);
  sys_return_if_fail(socket != NULL);

  self->socket = sys_object_ref(socket);
  self->laddr = *laddr;
  self->raddr = *raddr;
}

SysSocketConnection* sys_socket_connection_new_I(
    SysSocket *socket,
    SysSocketAddrIn *laddr,
    SysSocketAddrIn *raddr) {
  sys_return_val_if_fail(socket != NULL, NULL);
  sys_return_val_if_fail(laddr != NULL, NULL);
  sys_return_val_if_fail(raddr != NULL, NULL);

  SysSocketConnection* o = sys_socket_connection_new();

  sys_socket_connection_construct(o, laddr, raddr, socket);

  return o;
}

SysSocketAddrIn* sys_socket_connection_get_laddr(SysSocketConnection* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  return &self->laddr;
}

SysSocketAddrIn* sys_socket_connection_get_raddr(SysSocketConnection* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  return &self->raddr;
}

SysSocket* sys_socket_connection_get_socket(SysSocketConnection* self) {
  sys_return_val_if_fail(self != NULL, NULL);

  return self->socket;
}

/* object api */
static void sys_socket_connection_dispose(SysObject* o) {
  SysSocketConnection *self = SYS_SOCKET_CONNECTION(o);

  sys_clear_pointer(&self->socket, _sys_object_unref);

  SYS_OBJECT_CLASS(sys_socket_connection_parent_class)->dispose(o);
}

SysSocketConnection *sys_socket_connection_new(void) {
  return sys_object_new(SOCKET_TYPE_CONNECTION, NULL);
}

static void sys_socket_connection_class_init(SysSocketConnectionClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_socket_connection_dispose;
}

static void sys_socket_connection_init(SysSocketConnection *self) {
}
