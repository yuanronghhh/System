#include <System/Platform/Common/SysSocketAddr.h>
#include <System/Utils/SysError.h>
#include <System/Utils/SysStr.h>

static unsigned long get_inet_addr(const SysChar* host) {
  if (sys_str_equal(host, "localhost")) {
    return INADDR_LOOPBACK;
  }

  if (sys_str_equal(host, "0.0.0.0")) {
    return INADDR_ANY;
  }

  return inet_addr(host);
}

const SysChar *sys_socket_addr_get_host(SysSocketAddrIn *self) {

  return inet_ntoa(self->sin_addr);
}

int sys_socket_addr_get_port(SysSocketAddrIn *self) {

  return ntohs(self->sin_port);
}

SysChar* sys_socket_addr_to_string(SysSocketAddrIn *self) {
  const SysChar *host = inet_ntoa(self->sin_addr);
  int port = ntohs(self->sin_port);

  return sys_strdup_printf("%s:%d", host, port);
}

SysBool sys_socket_addr_equal(SysSocketAddrIn *a, SysSocketAddrIn *b) {
  return a->sin_port == b->sin_port
    && a->sin_addr.s_addr == b->sin_addr.s_addr;
}

static void sys_socket_addr_create_i(SysSocketAddrIn *self,
    int sin_family,
    const SysChar *host,
    int port) {

  self->sin_port = htons(port);
  self->sin_family = AF_INET;
  self->sin_addr.s_addr = htonl(get_inet_addr(host));
}

void sys_socket_addr_create_inet(SysSocketAddrIn *self,
    const SysChar *host,
    int port) {

  sys_socket_addr_create_i(self, AF_INET, host, port);
}
