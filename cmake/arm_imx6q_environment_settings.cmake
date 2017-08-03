include(CMakeForceCompiler)
# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm) 
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   arm-linux-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-g++)
SET(CMAKE_STRIP        arm-linux-strip)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


SET(VZPROJECT_INCLUDE_DIR
	${INTERFACE_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/src/lib
	${PROJECT_SOURCE_DIR}/src/third_part
	${PROJECT_SOURCE_DIR}/src/third_part/boost
	${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/imx6q/include
	${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/imx6q/include
)

MESSAGE(STATUS "       Add LIBRARY PATH AND NAME")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	SET(VZPROJECT_LIBRARY_DIR
		${INTERFACE_LIBRARY_DIR}
		${PROJECT_SOURCE_DIR}/lib/hi3518
		${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/imx6q/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/imx6q/lib
	)
	SET(VZPROJECT_LINK_LIB
		libvzlogging.so
		libevent_core.so
		libevent_pthreads.so
        -Wl,-rpath=.
		pthread
		rt
		dl
	)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       RELEASE MODE")
	MESSAGE(STATUS "       UNIX")
	SET(VZPROJECT_LIBRARY_DIR
		${INTERFACE_LIBRARY_DIR}
		${PROJECT_SOURCE_DIR}/lib/hi3518
		${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/imx6q/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/imx6q/lib
	)
	SET(VZPROJECT_LINK_LIB
		vzlogging
		libevent_core.so
		libevent_pthreads.so
        -Wl,-rpath=.
		pthread
		rt
		dl
	)
ENDIF()

ADD_DEFINITIONS(
    -DO3
    -D_LINUX
	-DPOSIX
	-DIMX6Q
)
    
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/imx6q")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/imx6q")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/imx6q")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/imx6q")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/imx6q")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/imx6q")

# With Release properties
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/imx6q")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/imx6q")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/imx6q")


