CMAKE_MINIMUM_REQUIRED ( VERSION 3.6 )

PROJECT ( Scheduler )

INCLUDE ( CheckCXXCompilerFlag )

# conan 사용을 위한 python interpreter 확인
FIND_PACKAGE ( PythonInterp )
# conan cmake 설치, conan 을 이용한 라이브러리 설치
IF ( NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake" )
  MESSAGE ( STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan" )
   FILE ( DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.13/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
ENDIF ()

INCLUDE ( ${CMAKE_BINARY_DIR}/conan.cmake )

conan_check(VERSION 1.17.2 REQUIRED)
conan_add_remote(NAME mpusz
                URL https://api.bintray.com/conan/mpusz/conan-mpusz)

#  conan_cmake_run ( CONANFILE conanfile.py BASIC_SETUP NO_OUTPUT_DIRS BUILD missing BUILD_TYPE Release VERBOSE)
# BASIC_SETUP 옵션을 줄경우 conan_cmake_run 내부에서 conan_basic_setup 이 실행된다.
#  conan_basic_setup 의 경우 output dirs 등의 전역 변수에 대한 설정이 실행되므로 
# BASIC_SETUP 의 경우 별도의 conan_basic_setup 으로 처리한다.
conan_cmake_run ( CONANFILE conanfile.py BUILD missing BUILD_TYPE Release VERBOSE)

# conan 을 이용하여 빌드된 라이브러리의 타겟 매크로 설정
# ARGUMENTS_NO_OUTPUT_DIRS 
#   설정을 하지 않을 경우 conan_cmake_run 내부에서 output 경로를 빌드되는 디렉토리 기준으로 전역 설정한다.
#   따라서 타겟별로 설정된 output 이 적용되지 않으므로, 타겟별로 output 을 별도 설정 한 경우 해당 옵션을 추가해야 한다.
conan_basic_setup ( TARGETS NO_OUTPUT_DIRS )

INCLUDE_DIRECTORIES ( ${CONAN_INCLUDE_DIRS} )


SET ( LIB_NAME "scheduler" )
SET ( TESTER_NAME "unit_test" )

FIND_PACKAGE ( Threads REQUIRED)

# c++11 standard support 체크를 컴파일러 플래그 체크 함수를 사용하도록 변경
CHECK_CXX_COMPILER_FLAG (-std=c++11 COMPILER_SUPPORT_CXX11 )
IF ( COMPILER_SUPPORT_CXX11 )
  SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g" )
ELSE ()
  # 컴파일러가 c++11 지원하지 않을 경우 빌드 불가
  MESSAGE ( FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support" )
ENDIF ()


ADD_CUSTOM_TARGET ( run
                    COMMAND ${TESTER_NAME}
)
ADD_CUSTOM_COMMAND ( TARGET run POST_BUILD
                    COMMAND ;
                    COMMENT "build & run" )

# libbase & external end
ADD_SUBDIRECTORY ( src )
ADD_SUBDIRECTORY ( test )

