#include <System/Platform/Common/SysSocketPrivate.h>

SysSocket *sys_socket_new(SOCKET fd) {
  SysSocket *s = sys_new0_N(SysSocket, 1);

  s->fd = fd;

  return s;
}

