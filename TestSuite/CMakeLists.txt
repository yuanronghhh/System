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
  ./tests/SysTestIFace.c
  ./tests/SysTestIFace.h
  ./tests/SysTestImpl.c
  ./tests/SysTestImpl.h

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
