set(INC
  .
  ./Fundamental
  ./DataTypes
  ./Platform
  ./Platform/Unix
  ./Platform/Win32
  ./Utils
)

set(INC_SYS
  ${PTHREAD_INCLUDE_DIRS}
  ${VLD_INCLUDE_DIRS}
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
)

if(UNIX)
  set(OS_SYS_SRC ${UNIX_SYS_SRC})
elseif(WIN32)
  set(OS_SYS_SRC ${WIN32_SYS_SRC})
endif()

set(SRC
  ${OS_SYS_SRC}

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

  ./Utils/SysFile.h
  ./Utils/SysFilePrivate.h
  ./Utils/SysFile.c
  ./Utils/SysStringPrivate.h
  ./Utils/SysString.h
  ./Utils/SysString.c
  ./Utils/SysPathPrivate.h
  ./Utils/SysPath.h
  ./Utils/SysPath.c
  ./Utils/SysError.h
  ./Utils/SysErrorPrivate.h
  ./Utils/SysError.c
  ./Utils/SysTypes.h
  ./Utils/SysTypes.c
  ./Utils/SysTextIO.h
  ./Utils/SysTextIO.c

  ./Platform/SysMemPrivate.h
  ./Platform/SysMem.h
  ./Platform/SysMemMap.h
  ./Platform/SysMem.c
  ./Platform/SysOsPrivate.h
  ./Platform/SysOs.h
  ./Platform/SysOs.c
  ./Platform/SysAtomic.h
  ./Platform/SysAtomicPrivate.h
  ./Platform/SysAtomic.c
  ./Platform/SysThread.h
  ./Platform/SysThreadPrivate.h
  ./Platform/SysThread.c
  ./Platform/SysProcess.c
  ./Platform/SysProcess.h
  ./Platform/SysProcessPrivate.h

  ./Fundamental/SysCommon.h
  ./Fundamental/SysCommonCore.h

  ./SysConfig.h
  ./SysConfig.h.in
  ./SysCore.h  # for expose api
)

add_dep_libs(System "${SRC}" "${INC}" "${INC_SYS}")

configure_file(${CMAKE_CURRENT_LIST_DIR}/SysConfig.h.in
  ${CMAKE_CURRENT_LIST_DIR}/SysConfig.h)

target_link_libraries(System
  ${PTHREAD_LIBRARIES}
)
set_property(TARGET System PROPERTY FOLDER CstProject)
