include(CMakeForceCompiler)
# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
set( CMAKE_SYSTEM_PROCESSOR arm ) 
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

#SET(LIB_CURL ${PROJECT_SOURCE_DIR}/src/third_part/curl)

SET(VZPROJECT_INCLUDE_DIR
	${INTERFACE_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/src/lib
	${PROJECT_SOURCE_DIR}/src/third_part
	${PROJECT_SOURCE_DIR}/src/third_part/boost
	${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/sunxiv40/include
	${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/sunxiv40/include
)

MESSAGE(STATUS "       Add LIBRARY PATH AND NAME")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	SET(VZPROJECT_LIBRARY_DIR
		${INTERFACE_LIBRARY_DIR}
		${PROJECT_SOURCE_DIR}/lib/sunxiv40
		${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/sunxiv40/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/sunxiv40/lib
	)
	SET(VZPROJECT_LINK_LIB
		libevent_core.so
		libevent_pthreads.so
		libvzlogging.so
        -Wl,-rpath=.
		pthread
		rt
	)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       RELEASE MODE")
	MESSAGE(STATUS "       UNIX")
	SET(VZPROJECT_LIBRARY_DIR
		${INTERFACE_LIBRARY_DIR}
		${PROJECT_SOURCE_DIR}/lib/sunxiv40
		${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/sunxiv40/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/sunxiv40/lib
	)
	SET(VZPROJECT_LINK_LIB
		vzlogging
		libevent_core.so
		libevent_pthreads.so
		-lm
		-ldl
        -Wl,-rpath=.
		pthread
		rt
	)
ENDIF()

#MESSAGE(STATUS "SETP 4 : ADD CODE SOURCE")

ADD_DEFINITIONS(
    -DO3
    -D_LINUX
	-DPOSIX
	#-mcpu=arm926ej-s
	-mno-unaligned-access
	-fno-aggressive-loop-optimizations
)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/sunxiv40")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/sunxiv40")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/sunxiv40")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/sunxiv40")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/sunxiv40")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/sunxiv40")

# With Release properties
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/sunxiv40")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/sunxiv40")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/sunxiv40")


