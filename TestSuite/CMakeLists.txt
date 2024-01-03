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
  ./Type.c
  ./Type.h
  ./Common.h
)

add_executable(SystemTestSuite "${SRC}")
set_property(TARGET SystemTestSuite PROPERTY FOLDER CstProject)

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
