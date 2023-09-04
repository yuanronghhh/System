#include <System/Platform/Common/SysSocket.h>

SysSocket *sys_socket_new(SysInt fd) {
  SysSocket *s = sys_new0_N(SysSocket, 1);

  s->fd = fd;

  return s;
}

