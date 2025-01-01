#ifndef __SYS_THIRD_PARTY_H__
#define __SYS_THIRD_PARTY_H__

#include <System/Fundamental/SysCommonBase.h>

SYS_BEGIN_DECLS

SYS_DIAGNOSTIC_PUSH
#include <System/ThirdParty/SysThirdWarning.h>

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

#if USE_LIBDWARF
#include <System/ThirdParty/SysLibDwarf.h>
#endif

SYS_DIAGNOSTIC_POP

SYS_END_DECLS

#endif

