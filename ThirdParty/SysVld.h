#ifndef __SYS_VLD_H__
#define __SYS_VLD_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

#if defined(USE_DEBUGGER)
  #if !defined(_DEBUG)
    #define _DEBUG SYS_DEBUG
  #endif 

  #pragma warning(push)
  #include <vld.h>
  #pragma warning(pop)
#endif


SYS_END_DECLS

#endif

