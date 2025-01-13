add_use_name(iconv 1)
add_use_name(openssl 1)
add_use_name(refcount 1)
add_use_name(marksweep 1)
add_use_name(vld 0)

set(INC
  ../
)

set(INC_SYS
  ${PTHREAD_INCLUDE_DIRS}
  ${VLD_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIRS}
  ${ICONV_INCLUDE_DIRS}
)

set(ANDROID_SYS_SRC
  ./Platform/Android/SysFileAndroid.c
  ./Platform/Android/SysStrAndroid.c
  ./Platform/Android/SysOsAndroid.c
  ./Platform/Android/SysMemAndroid.c
  ./Platform/Android/SysThreadAndroid.c
  ./Platform/Android/SysAndroid.h
  ./Platform/Android/SysErrorAndroid.c
  ./Platform/Android/SysAtomicAndroid.c
  ./Platform/Android/SysProcessAndroid.c
  ./Platform/Android/SysPathAndroid.c
  ./Platform/Android/SysSocketAndroid.c
)

set(UNIX_SYS_SRC
  ./Platform/Unix/SysFileUnix.c
  ./Platform/Unix/SysStrUnix.c
  ./Platform/Unix/SysOsUnix.c
  ./Platform/Unix/SysMemUnix.c
  ./Platform/Unix/SysThreadUnix.c
  ./Platform/Unix/SysUnix.h
  ./Platform/Unix/SysErrorUnix.c
  ./Platform/Unix/SysAtomicUnix.c
  ./Platform/Unix/SysProcessUnix.c
  ./Platform/Unix/SysPathUnix.c
  ./Platform/Unix/SysSocketUnix.c
)
set(WIN32_SYS_SRC
  ./Platform/Win32/SysFileWin32.c
  ./Platform/Win32/SysStrWin32.c
  ./Platform/Win32/SysOsWin32.c
  ./Platform/Win32/SysPathWin32.c
  ./Platform/Win32/SysErrorWin32.c
  ./Platform/Win32/SysMemWin32.c
  ./Platform/Win32/SysAtomicWin32.c
  ./Platform/Win32/SysThreadWin32.c
  ./Platform/Win32/SysWin32.h
  ./Platform/Win32/SysProcessWin32.c
  ./Platform/Win32/SysSocketWin32.c
)

if(ANDROID)
  set(OS_SYS_SRC ${ANDROID_SYS_SRC})
elseif(UNIX)
  set(OS_SYS_SRC ${UNIX_SYS_SRC})
elseif(WIN32)
  set(OS_SYS_SRC ${WIN32_SYS_SRC})
endif()

if(USE_OPENSSL)
  set(OPENSSL_SRC
    ./ThirdParty/SysOpenssl.h
    ./ThirdParty/SysOpenssl.c
  )

  LIST(APPEND OS_SYS_SRC ${OPENSSL_SRC})
endif()

if(USE_ICONV)
  set(ICONV_SRC
    ./ThirdParty/SysIconv.h
    ./ThirdParty/SysIconv.c
    ./ThirdParty/SysLibCharSet.h
    ./ThirdParty/SysLibCharSet.c
    ./ThirdParty/localcharset.h
    ./ThirdParty/localcharset.c
    ./ThirdParty/libcharset.h
  )

  LIST(APPEND OS_SYS_SRC ${ICONV_SRC})
endif()

if(USE_VLD)
  set(VLD_SRC
    ./ThirdParty/SysVld.h
    ./ThirdParty/SysVld.c
  )

  LIST(APPEND OS_SYS_SRC ${VLD_SRC})
endif()

if(USE_MARKSWEEP)
  set(MARKSWEEP_SRC
    ./Type/MarkSweep/SysMarkSweep.c
    ./Type/MarkSweep/SysMarkSweep.h
    ./Type/MarkSweep/SysMsBlock.c
    ./Type/MarkSweep/SysMsBlock.h
    ./Type/MarkSweep/SysMsMap.c
    ./Type/MarkSweep/SysMsMap.h
  )

  LIST(APPEND OS_SYS_SRC ${MARKSWEEP_SRC})
endif()

if(USE_REFCOUNT)
  set(REFCOUNT_SRC
    ./Type/Ref/SysRefBlock.c
    ./Type/Ref/SysRefBlock.h
    ./Type/Ref/SysRefGc.c
    ./Type/Ref/SysRefGc.h
  )

  LIST(APPEND OS_SYS_SRC ${REFCOUNT_SRC})
endif()

set(SRC
  ${OS_SYS_SRC}

  ./DataTypes/SysBit.h
  ./DataTypes/SysNode.h
  ./DataTypes/SysNode.c
  ./DataTypes/SysHNode.h
  ./DataTypes/SysHNode.c
  ./DataTypes/SysTree.h
  ./DataTypes/SysTree.c
  ./DataTypes/SysHArray.h
  ./DataTypes/SysHArray.c
  ./DataTypes/SysBHeap.h
  ./DataTypes/SysBHeap.c
  ./DataTypes/SysHashTable.h
  ./DataTypes/SysHashTable.c
  ./DataTypes/SysArray.h
  ./DataTypes/SysArray.c
  ./DataTypes/SysList.h
  ./DataTypes/SysList.c
  ./DataTypes/SysHList.h
  ./DataTypes/SysHList.c
  ./DataTypes/SysHCommon.h
  ./DataTypes/SysHCommon.c
  ./DataTypes/SysSList.h
  ./DataTypes/SysSList.c
  ./DataTypes/SysHsList.h
  ./DataTypes/SysHsList.c
  ./DataTypes/SysQueue.h
  ./DataTypes/SysQueue.c
  ./DataTypes/SysHQueue.h
  ./DataTypes/SysHQueue.c
  ./DataTypes/SysAsyncQueue.h
  ./DataTypes/SysAsyncQueue.c
  ./DataTypes/SysPQueue.h
  ./DataTypes/SysPQueue.c
  ./DataTypes/SysValue.h
  ./DataTypes/SysValue.c
  ./DataTypes/SysQuark.h
  ./DataTypes/SysQuark.c
  ./DataTypes/SysString.c
  ./DataTypes/SysString.h
  ./DataTypes/SysBytes.c
  ./DataTypes/SysBytes.h

  ./Type/SysTypeCommon.h
  ./Type/SysTypeCommon.c
  ./Type/SysType.h
  ./Type/SysTypePrivate.h
  ./Type/SysType.c
  ./Type/SysBlock.h
  ./Type/SysBlockPrivate.h
  ./Type/SysBlock.c
  ./Type/SysGcCommon.c
  ./Type/SysGcCommon.h
  ./Type/SysGcCommonPrivate.h
  ./Type/SysGcCore.c
  ./Type/SysGcCore.h
  ./Type/SysObject.h
  ./Type/SysObject.c
  ./Type/SysParam.h
  ./Type/SysParam.c

  ./Utils/SysUtf8.h
  ./Utils/SysUtf8.c
  ./Utils/SysUnicode.h
  ./Utils/SysUnicode.c
  ./Utils/SysUnicodeDecomp.h
  ./Utils/SysUnicodeDecomp.c
  ./Utils/SysUnicodeComp.h
  ./Utils/SysUnicodeMirrorTable.h
  ./Utils/SysUnicodeScriptTable.h
  ./Utils/SysUnicodeTable.h
  ./Utils/SysConvert.h
  ./Utils/SysConvert.c
  ./Utils/SysStr.h
  ./Utils/SysStr.c
  ./Utils/SysEnum.h
  ./Utils/SysEnum.c
  ./Utils/SysPathPrivate.h
  ./Utils/SysPath.h
  ./Utils/SysPath.c
  ./Utils/SysError.h
  ./Utils/SysErrorPrivate.h
  ./Utils/SysError.c
  ./Utils/SysTextIO.h
  ./Utils/SysTextIO.c
  ./Utils/SysSsl.h
  ./Utils/SysSsl.c
  ./Utils/SysCharSet.h
  ./Utils/SysCharSet.c

  ./Platform/Common/SysTimeZone.h
  ./Platform/Common/SysTimeZone.c
  ./Platform/Common/SysDateTime.h
  ./Platform/Common/SysDateTime.c
  ./Platform/Common/SysDate.h
  ./Platform/Common/SysDate.c
  ./Platform/Common/SysFile.h
  ./Platform/Common/SysFile.c
  ./Platform/Common/SysMemPrivate.h
  ./Platform/Common/SysMem.h
  ./Platform/Common/SysMemMap.h
  ./Platform/Common/SysMem.c
  ./Platform/Common/SysOsPrivate.h
  ./Platform/Common/SysOs.h
  ./Platform/Common/SysOs.c
  ./Platform/Common/SysAtomic.h
  ./Platform/Common/SysAtomic.c
  ./Platform/Common/SysRefCountPrivate.h
  ./Platform/Common/SysRefCount.c
  ./Platform/Common/SysThread.c
  ./Platform/Common/SysThread.h
  ./Platform/Common/SysThreadPrivate.h
  ./Platform/Common/SysSocket.c
  ./Platform/Common/SysSocket.h
  ./Platform/Common/SysSocketPrivate.h

  ./Platform/Common/SysProcess.c
  ./Platform/Common/SysProcess.h
  ./Platform/Common/SysProcessPrivate.h

  ./Fundamental/SysCommonCore.h
  ./Fundamental/SysCommon.h
  ./Fundamental/SysCommon.c
  ./Fundamental/SysCommonBase.h
  ./Fundamental/SysCommonTypes.h
  ./ThirdParty/SysThirdParty.h
  ./ThirdParty/SysThirdWarning.h

  ./SysConfig.h
  ./SysConfig.h.in
  ./SysCore.h  # for expose api
  ./SysCore.c
)

add_dep_libs(System "${SRC}" "${INC}" "${INC_SYS}")
target_copy_release_files(System)

configure_file(${CMAKE_CURRENT_LIST_DIR}/SysConfig.h.in
  ${CMAKE_CURRENT_LIST_DIR}/SysConfig.h)

target_link_libraries(System
  ${PTHREAD_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${ADDTIONAL_LIBRARIES}
  ${DBG_LIBRAREIS}
  ${VLD_LIBRARIES}
  ${ANDROID_LIBRARIES}
  ${ICONV_LIBRARIES}
)
set_property(TARGET System PROPERTY FOLDER CstProject)
