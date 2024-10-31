#ifndef __SYS_CORE_H__
#define __SYS_CORE_H__

#include <System/SysConfig.h>
#include <System/Fundamental/SysCommon.h>
#include <System/Fundamental/SysCommonCore.h>

#include <System/Platform/Common/SysOs.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Platform/Common/SysAtomic.h>
#include <System/Platform/Common/SysProcess.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysSocket.h>
#include <System/Platform/Common/SysFile.h>
#include <System/Platform/Common/SysDate.h>
#include <System/Platform/Common/SysTimeZone.h>
#include <System/Platform/Common/SysMappedFile.h>

#include <System/DataTypes/SysBit.h>
#include <System/DataTypes/SysQuark.h>
#include <System/DataTypes/SysClouse.h>
#include <System/DataTypes/SysArray.h>
#include <System/DataTypes/SysString.h>
#include <System/DataTypes/SysBytes.h>
#include <System/DataTypes/SysHArray.h>
#include <System/DataTypes/SysValue.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/DataTypes/SysList.h>
#include <System/DataTypes/SysSList.h>
#include <System/DataTypes/SysHsList.h>
#include <System/DataTypes/SysQueue.h>
#include <System/DataTypes/SysAsyncQueue.h>
#include <System/DataTypes/SysBHeap.h>
#include <System/DataTypes/SysNode.h>
#include <System/DataTypes/SysHNode.h>
#include <System/DataTypes/SysTree.h>
#include <System/DataTypes/SysPQueue.h>

#include <System/Type/SysType.h>
#include <System/Type/SysObject.h>
#include <System/Type/SysParam.h>
#include <System/Type/SysBlock.h>
#include <System/Type/SysTypeCommon.h>

#include <System/Utils/SysError.h>
#include <System/Utils/SysError.h>
#include <System/Utils/SysPath.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysEnum.h>
#include <System/Utils/SysTextIO.h>

#if USE_OPENSSL
#include <System/Utils/SysSsl.h>
#endif

SYS_API void sys_setup(void);
SYS_API void sys_teardown(void);

#endif
