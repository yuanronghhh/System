#include <System/Utils/SysSocket.h>


SysSocket *sys_socket(int domain, int type, int protocol) {
  SysSocket *s = sys_new0_N(SysSocket, 1);

  s->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (s->server_socket == INVALID_SOCKET) {
    return NULL;
  }

  return s;
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

int sys_listen(SysSocket *s, int backlog) {
  sys_return_val_if_fail(s != NULL, -1);

  return listen(s->fd, backlog);
}

SysSocket* sys_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen, int flags) {
  sys_return_val_if_fail(s != NULL, NULL);

  s->fd = accept(s, addr, addrlen, flags);
  if(s->fd < 0) {
    return NULL;
  }

  return s;
}

int sys_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  return bind(s->fd, addr, len);
}

int sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  return getaddrinfo (node, service, hints, res);
}
