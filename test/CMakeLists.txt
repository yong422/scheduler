# test cmake

ENABLE_TESTING ()

INCLUDE_DIRECTORIES ( ../src/ykjo )
SET ( TEST_DIRS
      ./
)
FOREACH ( dir ${TEST_DIRS} )
  FILE (GLOB TEMP_FILES "${dir}/*.cpp" )
  SET ( TEST_SRCS ${TEST_SRCS} ${TEMP_FILES} )
ENDFOREACH()

ADD_EXECUTABLE ( ${TESTER_NAME} ${TEST_SRCS} )
ADD_DEPENDENCIES ( ${TESTER_NAME} ${LIB_NAME} )

TARGET_LINK_LIBRARIES ( ${TESTER_NAME}
                        ${LIB_NAME}
                        CONAN_PKG::gtest
                        ${CMAKE_THREAD_LIBS_INIT} )

ADD_TEST ( NAME ${TESTER_NAME}
           COMMAND ${TESTER_NAME} )

