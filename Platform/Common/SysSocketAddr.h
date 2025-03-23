#ifndef __SYS_SOCKET_ADDR_H__
#define __SYS_SOCKET_ADDR_H__

#include <System/Platform/Common/SysSocketCommon.h>

SYS_BEGIN_DECLS

int sys_socket_addr_get_port(SysSocketAddrIn *self);

const SysChar *sys_socket_addr_get_host(SysSocketAddrIn *self);

SysBool sys_socket_addr_equal(SysSocketAddrIn *a, SysSocketAddrIn *b);

void sys_socket_addr_create_inet(SysSocketAddrIn *self,
    const SysChar *host,
    SysInt port);

const SysChar* sys_socket_addr_to_string(SysSocketAddrIn *self);

SYS_END_DECLS

#endif
