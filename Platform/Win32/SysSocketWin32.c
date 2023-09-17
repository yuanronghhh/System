#include <System/Platform/Common/SysSocketPrivate.h>
#include <System/DataTypes/SysQuark.h>
#include <System/Utils/SysString.h>

SysSocket *sys_socket_new_I(int domain, int type, int protocol, SysBool noblocking) {
  SOCKET fd;

  fd = socket(domain, type, protocol);

  if (fd == INVALID_SOCKET) {
    sys_debug_N("socket: %s", sys_socket_strerror(sys_socket_errno()));
    return NULL;
  }

  return sys_socket_new_fd(fd);
}

void sys_socket_free(SysSocket *s) {
  sys_return_if_fail(s != NULL);

#if USE_OPENSSL
  SSL_shutdown(s->ssl);
  SSL_free(s->ssl);
#else
  shutdown(s->fd, SD_BOTH);
  closesocket(s->fd);
#endif

  sys_free_N(s);
}

int sys_socket_setopt(SysSocket *s, int level, int optname, void *optval, socklen_t optlen) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = setsockopt(s->fd, level, optname, (char *)optval, optlen);
  if (r < 0) {

    sys_debug_N("setsockopt: %s", sys_socket_strerror(sys_socket_errno()));
  }

  return r;
}

int sys_socket_listen(SysSocket *s, int backlog) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = listen(s->fd, backlog);
  if (r < 0) {

    sys_debug_N("listen: %s", sys_socket_strerror(sys_socket_errno()));
  }

  return r;
}

SysSocket* sys_socket_accept(SysSocket *s, struct sockaddr *addr, socklen_t *addrlen) {
  sys_return_val_if_fail(s != NULL, NULL);
  SOCKET fd;

  fd = accept(s->fd, addr, addrlen);
  if(fd == INVALID_SOCKET) {

    sys_debug_N("accept: %s", sys_socket_strerror(sys_socket_errno()));
    return NULL;
  }

#if USE_OPENSSL
  if (SSL_accept(s->ssl) <= 0) {
    return NULL;
  }
#endif

  return sys_socket_new_fd((SysInt)fd);
}

int sys_socket_bind(SysSocket* s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = bind(s->fd, addr, addrlen);
  if (r < 0) {

    sys_debug_N("bind: %s", sys_socket_strerror(sys_socket_errno()));
  }

  return r;
}

int sys_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  return getaddrinfo (node, service, hints, res);
}

void sys_freeaddrinfo(struct addrinfo *res) {
  sys_return_if_fail(res != NULL);

  freeaddrinfo(res);
}

int sys_socket_real_connect(SysSocket *s, const struct sockaddr *addr, socklen_t addrlen) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = connect(s->fd, addr, addrlen);
  if (r == SOCKET_ERROR) {

    sys_debug_N("connect: %s", sys_socket_error());
  }

#if USE_OPENSSL
    if (SSL_connect(s->ssl) <= 0) {
      sys_ssl_error();
      return -1;
    }

    if (SSL_get_verify_result(s->ssl) != X509_V_OK) {
      sys_ssl_error();
      return -1;
    }
#endif

  return r;
}

SOCKET sys_socket_get_fd(SysSocket *s) {
  sys_return_val_if_fail(s != NULL, -1);

  return s->fd;
}

int sys_socket_recv(SysSocket *s, void *buf, size_t len, int flags) {
  sys_return_val_if_fail(s != NULL, -1);
  int r;

#if USE_OPENSSL
  r = SSL_read(s->ssl, buf, (int)len);
#else
  r = recv(s->fd, buf, (int)len, flags);
#endif

  if (r < 0) {

    sys_debug_N("recv: %s", sys_socket_strerror(sys_socket_errno()));
  }
  return r;
}

int sys_socket_send(SysSocket *s, const void *buf, size_t len, int flags) {
  sys_return_val_if_fail(s != NULL, -1);
  int r;

#if USE_OPENSSL
  r = SSL_write(s->ssl, buf, (int)len);
#else
  r = send(s->fd, buf, (int)len, flags);
#endif
  if (r < 0) {

    sys_debug_N("send: %s", sys_socket_strerror(sys_socket_errno()));
  }
  return r;
}

SysInt sys_socket_ioctl(SysSocket *s, long cmd, u_long * argp) {
  sys_return_val_if_fail(s != NULL, -1);

  int r = ioctlsocket(s->fd, cmd, argp);
  if (r < 0) {

    sys_debug_N("ioctlsocket: %s", sys_socket_strerror(sys_socket_errno()));
  }
  return r;
}

SysInt sys_socket_errno(void) {
  return WSAGetLastError();
}

const char* sys_socket_strerror(int err) {
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

