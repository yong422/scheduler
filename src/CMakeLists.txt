INCLUDE_DIRECTORIES ( ./ykjo )
SET ( SRC_DIRS
      ./ykjo
)
FOREACH ( dir ${SRC_DIRS} )
  FILE (GLOB TEMP_FILES "${dir}/*.cpp" )
  SET ( SOURCES ${SOURCES} ${TEMP_FILES} )
ENDFOREACH()
ADD_LIBRARY ( ${LIB_NAME} ${SOURCES} )

TARGET_LINK_LIBRARIES ( ${LIB_NAME} 
                        ${CMAKE_THREAD_LIBS_INIT} )

# 빌드된 static library file 의 output 경로 설정
SET_TARGET_PROPERTIES ( ${LIB_NAME}
                          PROPERTIES
                          ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib )
