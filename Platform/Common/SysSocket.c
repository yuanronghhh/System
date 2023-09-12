#include <System/Platform/Common/SysSocketPrivate.h>

SysSocket *sys_socket_new(SOCKET fd, SysBool noblocking) {
  SysSocket *s = sys_new0_N(SysSocket, 1);

  s->fd = fd;
  s->noblocking = noblocking;
  sys_socket_set_noblock(s, noblocking);

  return s;
}

SysInt sys_socket_set_noblock(SysSocket *s, SysBool bvalue) {
  SysULong ul = bvalue;

  return sys_socket_ioctl(s, FIONBIO, &ul);
}

