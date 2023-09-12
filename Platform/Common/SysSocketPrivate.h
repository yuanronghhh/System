#ifndef __SYS_SOCKET_PRIVATE_H__
#define __SYS_SOCKET_PRIVATE_H__

#include <System/Platform/Common/SysSocket.h>

SYS_BEGIN_DECLS

struct _SysSocket {
  /* < private > */
#if SYS_OS_WIN32
  SOCKET fd;
#elif SYS_OS_UNIX
  SysInt fd;
#endif

  SysBool noblocking;

#if USE_OPENSSL
  SSL *ssl;
#endif
};

SYS_END_DECLS

#endif
