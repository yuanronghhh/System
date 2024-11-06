#ifndef __SYS_OPENSSL_H__
#define __SYS_OPENSSL_H__

#include <System/Fundamental/SysCommonBase.h>

SYS_BEGIN_DECLS

#include <openssl/ssl.h>
#include <openssl/err.h>

SYS_API void sys_ssl_setup(void);
SYS_API void sys_ssl_teardown(void);

SYS_API const SysChar * sys_ssl_error(void);

SYS_API void sys_ssl_ctx_setup(
    const SysChar* server_crt,
    const SysChar* server_priv,
    const SysChar* client_ca,
    const SysChar* client_priv);

SYS_API void sys_ssl_ctx_teardown(void);

SYS_API SSL_CTX* sys_ssl_ctx_get_server(void);
SYS_API SSL_CTX* sys_ssl_ctx_get_client(void);
SYS_API SysInt sys_ssl_renegotiate(SSL* ssl);

SYS_END_DECLS

#endif

