include(CMakeForceCompiler)
# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
set( CMAKE_SYSTEM_PROCESSOR arm ) 
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/hisi-linux/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-gcc)
SET(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-g++)
SET(CMAKE_STRIP        /opt/hisi-linux/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-strip)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#SET(LIB_CURL ${PROJECT_SOURCE_DIR}/src/third_part/curl)

SET(VZPROJECT_INCLUDE_DIR
	${PROJECT_SOURCE_DIR}/src/lib/
	${PROJECT_SOURCE_DIR}/src/third_sdk/libevent/WIN32-Code/nmake
	${PROJECT_SOURCE_DIR}/src/third_sdk/libevent/include
)

MESSAGE(STATUS "       Add LIBRARY PATH AND NAME")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	SET(VZPROJECT_LIBRARY_DIR
		${PROJECT_SOURCE_DIR}/lib/hi3516
		${PROJECT_SOURCE_DIR}/src/third_sdk/libevent/lib/hi3516
	)
	SET(VZPROJECT_LINK_LIB
        -lcrypto
        -Wl,-rpath=.
		pthread
		rt
	)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       RELEASE MODE")
	MESSAGE(STATUS "       UNIX")
	SET(VZPROJECT_LIBRARY_DIR
		${PROJECT_SOURCE_DIR}/lib/hi3516
		${PROJECT_SOURCE_DIR}/src/third_sdk/libevent/lib/hi3516
	)
	SET(VZPROJECT_LINK_LIB
        -lcrypto
        -Wl,-rpath=.
		pthread
		rt
	)
ENDIF()

#MESSAGE(STATUS "SETP 4 : ADD CODE SOURCE")

ADD_DEFINITIONS(
    -DO3
    -D_LINUX
)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/hi3516")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/hi3516")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/hi3516")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/hi3516")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/hi3516")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/hi3516")

# With Release properties
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/hi3516")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/hi3516")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/hi3516")


