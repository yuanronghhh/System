#ifndef __SYS_SSL_H__
#define __SYS_SSL_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

struct _SysSsl {
  SSL *ssl;
};

SYS_END_DECLS

#endif
