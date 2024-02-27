#include <System/Platform/Common/SysSocketPrivate.h>
#include <System/DataTypes/SysQuark.h>
#include <System/Utils/SysString.h>

SysSocket *sys_socket_real_new_I(SysInt domain, SysInt type, SysInt protocol, SysBool noblocking) {
  SOCKET fd;

  fd = socket(domain, type, protocol);

  if (fd == INVALID_SOCKET) {
    return NULL;
  }

  return sys_socket_new_fd(fd);
}

void sys_socket_real_close(SysSocket *s) {
  sys_return_if_fail(s != NULL);

  closesocket(s->fd);
}

SysInt sys_socket_setopt(SysSocket *s, SysInt level, SysInt optname, void *optval, socklen_t optlen) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = setsockopt(s->fd, level, optname, (char *)optval, optlen);
  if (r < 0) {

  }

  return r;
}

SysInt sys_socket_listen(SysSocket *s, SysInt backlog) {
  sys_return_val_if_fail(s != NULL, -1);

  return listen(s->fd, backlog);
}

SysSocket* sys_socket_real_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SOCKET fd;

  fd = accept(s->fd, addr, addrlen);
  if(fd == INVALID_SOCKET) {

    return NULL;
  }

  return sys_socket_new_fd(fd);
}

SysInt sys_socket_real_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = bind(s->fd, addr, addrlen);
  if (r < 0) {

  }

  return r;
}

SysInt sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  return getaddrinfo (node, service, hints, res);
}

void sys_freeaddrinfo(struct addrinfo *res) {
  sys_return_if_fail(res != NULL);

  freeaddrinfo(res);
}

SysInt sys_socket_real_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  SysInt r = connect(s->fd, addr, addrlen);
  if (r == SOCKET_ERROR) {

  }

  return r;
}

SysInt sys_socket_real_recv(SysSocket *s, void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return recv(s->fd, buf, (int)len, flags);
}

SysInt sys_socket_real_send(SysSocket *s, const void *buf, size_t len, SysInt flags) {
  sys_return_val_if_fail(s != NULL, -1);

  return send(s->fd, buf, (int)len, flags);
}

SysInt sys_socket_real_ioctl(SysSocket *s, long cmd, u_long * argp) {
  sys_return_val_if_fail(s != NULL, -1);

  return ioctlsocket(s->fd, cmd, argp);
}

SysInt sys_socket_errno(void) {
  return WSAGetLastError();
}

const char* sys_socket_strerror(SysInt err) {
  char *umsg;
  const char *qmsg;
  wchar_t *msg;

  FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER
    | FORMAT_MESSAGE_IGNORE_INSERTS
    | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, err, 0,
    (LPWSTR)&msg, 0, NULL);

  if (msg == NULL) {
    return NULL;
  }

  umsg = sys_wchar_to_mbyte(msg, NULL);
  if(umsg == NULL) {
    return NULL;
  }

  qmsg = sys_quark_string(umsg);
  LocalFree(msg);
  LocalFree(umsg);

  return qmsg;
}

