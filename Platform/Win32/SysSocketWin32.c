#include <System/Platform/Common/SysSocket.h>


SysSocket *sys_socket(int domain, int type, int protocol) {
  SOCKET fd;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == INVALID_SOCKET) {
    return NULL;
  }

  return sys_socket_new((SysInt)fd);
}

void sys_socket_free(SysSocket *s) {
  sys_return_if_fail(s != NULL);

  closesocket(s->fd);
  sys_free_N(s);
}

int sys_socket_setopt(SysSocket *s, int level, int optname, void *optval, socklen_t optlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return setsockopt(s->fd, level, optname, (char *)optval, optlen);
}

int sys_socket_listen(SysSocket *s, int backlog) {
  sys_return_val_if_fail(s != NULL, -1);

  return listen(s->fd, backlog);
}

SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SOCKET fd;

  fd = accept(s->fd, addr, addrlen);
  if(fd == INVALID_SOCKET) {

    return NULL;
  }

  return sys_socket_new((SysInt)fd);
}

int sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return bind(s->fd, addr, addrlen);
}

int sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  return getaddrinfo (node, service, hints, res);
}

void sys_freeaddrinfo(struct addrinfo *res) {
  sys_return_if_fail(res != NULL);

  freeaddrinfo(res);
}

SysSSize sys_socket_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return connect(s->fd, addr, addrlen);
}

SysInt sys_socket_get_fd(SysSocket *s) {
  sys_return_val_if_fail(s != NULL, -1);

  return (SysInt)s->fd;
}

SysSSize sys_socket_recv(SysSocket *s, void *buf, size_t len, int flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return recv(s->fd, buf, (int)len, flags);
}

SysSSize sys_socket_send(SysSocket *s, const void *buf, size_t len, int flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return send(s->fd, buf, (int)len, flags);
}
