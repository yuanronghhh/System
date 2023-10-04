set(INC
  ../
)

set(INC_SYS
  ${PTHREAD_INCLUDE_DIRS}
  ${VLD_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIRS}
)

set(UNIX_SYS_SRC
  ./Platform/Unix/SysFileUnix.c
  ./Platform/Unix/SysStringUnix.c
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
  ./Platform/Win32/SysStringWin32.c
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

if(UNIX)
  set(OS_SYS_SRC ${UNIX_SYS_SRC})
elseif(WIN32)
  set(OS_SYS_SRC ${WIN32_SYS_SRC})
endif()

set(SRC
  ${OS_SYS_SRC}

  ./DataTypes/SysNode.h
  ./DataTypes/SysNode.c
  ./DataTypes/SysTree.h
  ./DataTypes/SysTree.c

  ./DataTypes/SArray.h
  ./DataTypes/SArray.c

  ./DataTypes/SysBHeap.h
  ./DataTypes/SysBHeap.c
  ./DataTypes/SysHashTable.h
  ./DataTypes/SysHashTable.c
  ./DataTypes/SysArray.h
  ./DataTypes/SysArray.c
  ./DataTypes/SysList.h
  ./DataTypes/SysList.c
  ./DataTypes/SysSList.h
  ./DataTypes/SysSList.c
  ./DataTypes/SysQueue.h
  ./DataTypes/SysQueue.c
  ./DataTypes/SysValue.h
  ./DataTypes/SysValue.c
  ./DataTypes/SysTypes.h
  ./DataTypes/SysTypes.c
  ./DataTypes/SysQuark.h
  ./DataTypes/SysQuark.c

  ./Utils/SysFile.h
  ./Utils/SysFile.c
  ./Utils/SysString.h
  ./Utils/SysString.c
  ./Utils/SysPathPrivate.h
  ./Utils/SysPath.h
  ./Utils/SysPath.c
  ./Utils/SysError.h
  ./Utils/SysErrorPrivate.h
  ./Utils/SysError.c
  ./Utils/SysTextIO.h
  ./Utils/SysTextIO.c

  ./Platform/Common/SysMemPrivate.h
  ./Platform/Common/SysMem.h
  ./Platform/Common/SysMemMap.h
  ./Platform/Common/SysMem.c
  ./Platform/Common/SysOsPrivate.h
  ./Platform/Common/SysOs.h
  ./Platform/Common/SysOs.c
  ./Platform/Common/SysAtomic.h
  ./Platform/Common/SysAtomic.c
  ./Platform/Common/SysThread.c
  ./Platform/Common/SysThread.h
  ./Platform/Common/SysSocket.c
  ./Platform/Common/SysSocket.h
  ./Platform/Common/SysSocketPrivate.h
  ./Platform/Common/SysThreadPrivate.h

  ./Platform/Common/SysProcess.c
  ./Platform/Common/SysProcess.h
  ./Platform/Common/SysProcessPrivate.h

  ./Fundamental/SysCommonCore.h
  ./Fundamental/SysCommon.h

  ./SysConfig.h
  ./SysConfig.h.in
  ./SysCore.h  # for expose api
)

if(USE_OPENSSL)
  LIST(APPEND SRC
    ./Utils/SysOpenSSL.h
    ./Utils/SysOpenSSL.c)
endif()


add_dep_libs(System "${SRC}" "${INC}" "${INC_SYS}")

configure_file(${CMAKE_CURRENT_LIST_DIR}/SysConfig.h.in
  ${CMAKE_CURRENT_LIST_DIR}/SysConfig.h)

target_link_libraries(System
  ${OPENSSL_LIBRARIES}
  ${ADDTIONAL_LIBRARIES}
  ${PTHREAD_LIBRARIES}
  ${DBG_LIBRAREIS}
  ${VLD_LIBRAREIS}
)
set_property(TARGET System PROPERTY FOLDER CstProject)
