#ifndef __SYS_OPENSSL_H__
#define __SYS_OPENSSL_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#include <openssl/ssl.h>
#include <openssl/err.h>

SYS_API void sys_ssl_setup(void);
SYS_API void sys_ssl_teardown(void);

SYS_API SSL_CTX* sys_ssl_create_server_ctx(const SysChar* signed_key, const SysChar* priv_key);
SYS_API SSL_CTX* sys_ssl_create_client_ctx(const SysChar* ca_key, const SysChar* priv_key);

SYS_END_DECLS

#endif

