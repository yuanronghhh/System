#include <System/Platform/Common/SysSocketAddr.h>
#include <System/Utils/SysError.h>
#include <System/Utils/SysStr.h>

static unsigned long get_inet_addr(int family, const SysChar* host) {
  if (sys_str_equal(host, "localhost")) {
    return INADDR_LOOPBACK;
  }

  if (sys_str_equal(host, "0.0.0.0")) {
    return INADDR_ANY;
  }

  unsigned long addrv;
  if(inet_pton(family, host, &addrv) != 1) { return 0; }

  return addrv;
}

const SysChar *sys_socket_addr_get_host(SysSocketAddrIn *self) {
  struct sockaddr_in *addr = &self->parent;

  if(inet_ntop(addr->sin_family,
        &addr->sin_addr, 
        self->storage,
        INET6_ADDRSTRLEN) == NULL) {

    return NULL;
  }

  return self->storage;
}

int sys_socket_addr_get_port(SysSocketAddrIn *self) {
  struct sockaddr_in *addr = (struct sockaddr_in *)self;

  return ntohs(addr->sin_port);
}

SysChar* sys_socket_addr_to_string(SysSocketAddrIn *self) {
  struct sockaddr_in *addr = (struct sockaddr_in *)self;

  const SysChar *host = sys_socket_addr_get_host(self);
  int port = ntohs(addr->sin_port);

  return sys_strdup_printf("%s:%d", host, port);
}

SysBool sys_socket_addr_equal(SysSocketAddrIn *a, SysSocketAddrIn *b) {
  struct sockaddr_in *aaddr = (struct sockaddr_in *)a;
  struct sockaddr_in *baddr = (struct sockaddr_in *)b;

  return aaddr->sin_port == baddr->sin_port
    && aaddr->sin_addr.s_addr == baddr->sin_addr.s_addr;
}

static void sys_socket_addr_create_i(SysSocketAddrIn *self,
    int sin_family,
    const SysChar *host,
    int port) {

  struct sockaddr_in *addr = (struct sockaddr_in *)self;

  addr->sin_port = htons(port);
  addr->sin_family = sin_family;
  addr->sin_addr.s_addr = get_inet_addr(addr->sin_family, host);
}

void sys_socket_addr_create_inet(SysSocketAddrIn *self,
    const SysChar *host,
    int port) {

  sys_socket_addr_create_i(self, AF_INET, host, port);
}
