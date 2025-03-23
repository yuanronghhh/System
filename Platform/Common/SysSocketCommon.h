#ifndef __SYS_SOCKET_COMMON_H__
#define __SYS_SOCKET_COMMON_H__

#include <System/Type/SysObject.h>
#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

typedef struct _SysSocketConnection SysSocketConnection;
typedef struct _SysSocketConnectionClass SysSocketConnectionClass;

typedef SysSSize (*SysSocketConnectionFunc)(SysSocketConnection *self, SysPointer user_data);
typedef struct sockaddr_in SysSocketAddrIn;

SYS_END_DECLS

#endif
