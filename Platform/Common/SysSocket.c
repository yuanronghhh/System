#include <System/Platform/Common/SysSocketPrivate.h>
#include <System/Platform/Common/SysSocketAddr.h>
#include <System/Utils/SysError.h>

SYS_DEFINE_TYPE(SysSocket, sys_socket, SYS_TYPE_OBJECT);

SysSocket *sys_socket_new_I(SysInt domain, SysInt type, SysInt protocol) {
  SysSocket *ns = sys_socket_real_new_I(domain, type, protocol);
  socklen_t addrlen;

  union {
    struct sockaddr_storage storage;
    struct sockaddr sa;
  } address;

  memset (&address, 0, sizeof (address));

  if (ns == NULL) {
    sys_warning_N("%s", sys_socket_error());

    return NULL;
  }

  addrlen = sizeof(address);
  if (getpeername (ns->fd, &address.sa, &addrlen) >= 0) {
    ns->can_write = true;
    ns->can_read = true;
  }

  return ns;
}

SysSocket *sys_socket_new_fd(SOCKET fd) {
  SysSocket *s = sys_socket_new();

  s->fd = fd;

  return s;
}

SysBool sys_socket_set_no_blocking(SysSocket *s, SysBool bvalue) {
  sys_return_val_if_fail(s != NULL, -1);

  if (!sys_socket_real_set_no_blocking(s, bvalue)) {

    sys_warning_N("%s",
        sys_socket_error());
  }

  return true;
}

SysInt sys_socket_send(SysSocket *s, const void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return sys_socket_real_send(s, buf, len, flags);
}

const SysChar *sys_socket_error(void) {

  return sys_socket_strerror(sys_socket_errno());
}

SysInt sys_socket_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);
  SysInt r;

  r = sys_socket_real_connect(s, addr, addrlen);
  if (r < 0) {

    sys_warning_N("%s, %s",
        sys_socket_addr_to_string((SysSocketAddrIn *)addr),
        sys_socket_error());
  }

  s->can_read = true;
  s->can_write = true;

  return r;
}

SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SysSocket* cs;

  cs = sys_socket_real_accept(s, addr, addrlen);
  if(cs == NULL) {

    sys_warning_N("%s", sys_socket_error());
    return NULL;
  }

  s->can_read = true;
  s->can_write = true;

  return cs;
}

SysInt sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = bind(s->fd, addr, addrlen);
  if (r < 0) {

    sys_warning_N("%d, %s", r, sys_socket_error());
  }

  return r;
}


SysInt sys_socket_read(SysSocket *s, void *buf, size_t len) {
  sys_return_val_if_fail(s != NULL, -1);
  SysInt r;

  r = sys_socket_real_read(s, buf, (SysInt)len);
  if (r < 0) {

    sys_warning_N("%d, %s", r, sys_socket_error());
  }

  return r;
}

SysInt sys_socket_recv(SysSocket *s, void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);
  SysInt r;

  r = sys_socket_real_recv(s, buf, (SysInt)len, flags);
  if (r < 0) {

    sys_warning_N("%d, %s", r, sys_socket_error());
  }

  return r;
}

SysInt sys_socket_ioctl(SysSocket *s, long cmd, u_long * argp) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = sys_socket_real_ioctl(s, cmd, argp);
  if (r < 0) {

    sys_warning_N("%d, %s", r, sys_socket_error());
  }
  return r;
}

SysBool sys_socket_is_connected(SysSocket *s) {

  return s->can_read || s->can_write;
}

void sys_socket_shutdown(SysSocket *s, SysInt flags) {

  sys_socket_real_shutdown(s, flags);
}

void sys_socket_close(SysSocket *s) {
  sys_socket_shutdown(s, SYS_SOCKET_SD_BOTH);
  sys_socket_real_close(s);

  s->can_read = false;
  s->can_write = false;
}

SOCKET sys_socket_get_fd(SysSocket *s) {
  sys_return_val_if_fail(s != NULL, -1);

  return s->fd;
}

/* object api */
SysSocket* sys_socket_new(void) {
  return sys_object_new(SYS_TYPE_SOCKET, NULL);
}

static void sys_socket_dispose(SysObject* o) {
  SysSocket *self = SYS_SOCKET(o);

  sys_socket_real_close(self);
  self->can_write = false;
  self->can_read = false;

  SYS_OBJECT_CLASS(sys_socket_parent_class)->dispose(o);
}

static void sys_socket_class_init(SysSocketClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_socket_dispose;
}

void sys_socket_init(SysSocket* self) {
}

