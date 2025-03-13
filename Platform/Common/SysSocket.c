#include <System/Platform/Common/SysSocketPrivate.h>
#include <System/Utils/SysError.h>

SYS_DEFINE_TYPE(SysSocket, sys_socket, SYS_TYPE_OBJECT);

SysSocket *sys_socket_new_I(SysInt domain, SysInt type, SysInt protocol) {
  SysSocket *ns = sys_socket_real_new_I(domain, type, protocol);

  if (ns == NULL) {
    sys_warning_N("socket: %s", sys_socket_error());

    return NULL;
  }

  return ns;
}

SysSocket *sys_socket_new_fd(SOCKET fd) {
  SysSocket *s = sys_socket_new();

  s->fd = fd;
  s->noblocking = false;

  return s;
}

SysInt sys_socket_set_blocking(SysSocket *s, SysBool bvalue) {
  SysULong ul = bvalue;

  return sys_socket_ioctl(s, FIONBIO, &ul);
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

    sys_warning_N("connect: %s", sys_socket_error());
  }

  return r;
}

SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SysSocket* cs;

  cs = sys_socket_real_accept(s, addr, addrlen);
  if(cs == NULL) {

    sys_warning_N("accept: %s", sys_socket_error());
    return NULL;
  }

  return cs;
}

SysInt sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = bind(s->fd, addr, addrlen);
  if (r < 0) {

    sys_warning_N("bind: %s", sys_socket_error());
  }

  return r;
}


SysInt sys_socket_read(SysSocket *s, void *buf, size_t len) {
  sys_return_val_if_fail(s != NULL, -1);
  SysInt r;

  r = sys_socket_real_read(s, buf, (SysInt)len);
  if (r < 0) {

    sys_warning_N("recv: %s", sys_socket_error());
  }

  return r;
}

SysInt sys_socket_recv(SysSocket *s, void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);
  SysInt r;

  r = sys_socket_real_recv(s, buf, (SysInt)len, flags);
  if (r < 0) {

    sys_warning_N("recv: %s", sys_socket_error());
  }

  return r;
}

SysInt sys_socket_ioctl(SysSocket *s, long cmd, u_long * argp) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = sys_socket_real_ioctl(s, cmd, argp);
  if (r < 0) {

    sys_warning_N("ioctlsocket: %s", sys_socket_error());
  }
  return r;
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

  SYS_OBJECT_CLASS(sys_socket_parent_class)->dispose(o);
}

static void sys_socket_class_init(SysSocketClass* cls) {
  SysObjectClass *ocls = SYS_OBJECT_CLASS(cls);

  ocls->dispose = sys_socket_dispose;
}

void sys_socket_init(SysSocket* self) {
}

