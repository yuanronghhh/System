#include <System/Platform/Common/SysSocketPrivate.h>
#include <System/Utils/SysError.h>

SysSocket *sys_socket_real_new_I(SysInt domain, SysInt type, SysInt protocol) {
  SysInt fd;

  fd = socket(domain, type, protocol);
  if (fd < 0) {

    return NULL;
  }

  return sys_socket_new_fd(fd);
}

void sys_socket_real_close(SysSocket *s) {
  sys_return_if_fail(s != NULL);

  close(s->fd);
}

SysInt sys_socket_setopt(SysSocket *s, SysInt level, SysInt optname, const void *optval, socklen_t optlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return setsockopt(s->fd, level, optname, optval, optlen);
}

SysInt sys_socket_listen(SysSocket *s, SysInt backlog) {
  sys_return_val_if_fail(s != NULL, -1);

  return listen(s->fd, backlog);
}

SysSocket* sys_socket_real_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SysInt fd;

  fd = accept(s->fd, addr, addrlen);
  if(fd < 0) {

    return NULL;
  }

  return sys_socket_new_fd(fd);
}

SysInt sys_socket_real_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return bind(s->fd, addr, addrlen);
}

SysInt sys_getaddrinfo(const SysChar *node, const SysChar *service, const struct addrinfo *hints, struct addrinfo **res) {

  return getaddrinfo (node, service, hints, res);
}

void sys_freeaddrinfo(struct addrinfo *res) {
  sys_return_if_fail(res != NULL);

  sys_freeaddrinfo(res);
}

SysInt sys_socket_real_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return (SysInt)connect(s->fd, addr, addrlen);
}

SysInt sys_socket_real_read(SysSocket *s, void *buf, size_t len) {
  sys_return_val_if_fail(s != NULL, -1);

  return (SysInt)read(s->fd, buf, len);
}

SysInt sys_socket_real_recv(SysSocket *s, void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return (SysInt)recv(s->fd, buf, len, flags);
}

SysInt sys_socket_real_send(SysSocket *s, const void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return (SysInt)send(s->fd, buf, len, flags);
}

SysInt sys_socket_real_ioctl(SysSocket *s, long cmd, u_long * argp) {
  sys_return_val_if_fail(s != NULL, -1);

  return ioctl(s->fd, cmd, argp);
}

const SysChar * sys_socket_strerror(SysInt err) {
  return sys_strerror(err);
}

SysInt sys_socket_errno(void) {
  return errno;
}
