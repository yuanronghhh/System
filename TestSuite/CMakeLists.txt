include_dep_dirs(
  ./
  ./tests/
)

set(INC_SYS
  ${UNITY_INCLUDE_DIRS}
  ${PTHREAD_INCLUDE_DIRS}
  ${VLD_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIRS}
)

set(SRC
  ./tests/SysTestIFace2.c
  ./tests/SysTestIFace2.h
  ./tests/SysTestIFace.c
  ./tests/SysTestIFace.h
  ./tests/SysTestImpl.c
  ./tests/SysTestImpl.h
  ./tests/SysTestImpl2.c
  ./tests/SysTestImpl2.h

  ./Main.c
  ./Main.h
  ./BHeap.c
  ./BHeap.h
  ./Type.c
  ./Type.h
  ./Socket.c
  ./Socket.h
  ./Common.h
)

add_executable(SystemTestSuite "${SRC}")
set_property(TARGET SystemTestSuite PROPERTY FOLDER CstProject)

configure_file(${CMAKE_CURRENT_LIST_DIR}/Common.h.in
  ${CMAKE_CURRENT_LIST_DIR}/Common.h)


target_link_libraries(SystemTestSuite
  System
  ${OPENSSL_LIBRARIES}
  ${ADDTIONAL_LIBRARIES}
  ${PTHREAD_LIBRARIES}
  ${DBG_LIBRAREIS}
  ${VLD_LIBRARIES}
  ${UNITY_LIBRARIES}
)

target_copy_files(SystemTestSuite
  "${OPENSSL_FILES}"
  "${VLD_FILES}"
  "${PTHREAD_FILES}"
  "${UNITY_FILES}"
)
