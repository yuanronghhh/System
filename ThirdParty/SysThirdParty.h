#ifndef __SYS_THIRD_PARTY_H__
#define __SYS_THIRD_PARTY_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#if USE_OPENSSL
#include <System/ThirdParty/SysOpenssl.h>
#endif

#if USE_VLD
#include <System/ThirdParty/SysVld.h>
#endif

#if USE_ICONV
#include <System/ThirdParty/SysIconv.h>
#include <System/ThirdParty/SysLibCharSet.h>
#endif

SYS_END_DECLS

#endif

