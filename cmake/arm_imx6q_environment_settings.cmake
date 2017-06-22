include(CMakeForceCompiler)
# this one is important
SET(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR arm) 
#this one not so much
SET(CMAKE_SYSTEM_VERSION   1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-fsl-linux-gnueabi-gcc)
SET(CMAKE_CXX_COMPILER /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-fsl-linux-gnueabi-g++)
SET(CMAKE_STRIP        /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/arm-fsl-linux-gnueabi-strip)
SET(FTP_DIR            /mnt/hgfs/shared/workspace/git_work/isaws/ftp_dir)
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(THIRD_SDK            ${PROJECT_SOURCE_DIR}/src//third_sdk/)

SET(VZPROJECT_INCLUDE_DIR
	${PROJECT_SOURCE_DIR}/src/lib/
	${THIRD_SDK}/libevent/include/
)

MESSAGE(STATUS "       Add LIBRARY PATH AND NAME")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	SET(VZPROJECT_LIBRARY_DIR
		${PROJECT_SOURCE_DIR}/lib/imx6q
	)
	
	SET(YHPROJECT_LINK_LIB
		#event_core
        	-Wl,-rpath=.
		pthread
		rt
	)
	
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       RELEASE MODE")
	MESSAGE(STATUS "       UNIX")
	SET(VZPROJECT_LIBRARY_DIR
		${PROJECT_SOURCE_DIR}/lib/imx6q/
		${THIRD_SDK}/libevent/lib/imx6q/
	)
	
	SET(YHPROJECT_LINK_LIB
        	#event_core
        	-Wl,-rpath=.
		pthread
		rt
	)
ENDIF()

MESSAGE(STATUS "SETP 4 : ADD CODE SOURCE")

ADD_DEFINITIONS(
    -DO2
    -D_LINUX
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


