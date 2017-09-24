
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

##########################################################
## Step 1 
SET(INTERFACE_INCLUDE_DIR     "E:/vz/sdk/interface_8127/inc")
SET(INTERFACE_LIBRARY_DIR     "E:/vz/sdk/interface_8127/lib")

##1. INCLUDE_DIRECTORES
SET(VZPROJECT_INCLUDE_DIR
	${INTERFACE_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/src/lib
	${PROJECT_SOURCE_DIR}/src/third_part
	${PROJECT_SOURCE_DIR}/src/third_part/boost
	${PROJECT_SOURCE_DIR}/src/third_part/libiconv/win32/include
	${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/vs2010/include
	${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/vs2010/include
)

MESSAGE(STATUS "       Add library path and name")
if(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	MESSAGE(STATUS "       MSVC12")
	SET(VZPROJECT_LIBRARY_DIR
		${INTERFACE_LIBRARY_DIR}
		${PROJECT_SOURCE_DIR}/lib/vs2013
		${PROJECT_SOURCE_DIR}/src/third_part/libiconv/win32/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/vs2010/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/vs2010/lib
	)
	SET(VZPROJECT_LINK_LIB
		libevent_core.lib
		libvzlogging
		Iphlpapi.lib
		ws2_32.lib
		winmm.lib
	)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       Release Mode")
	MESSAGE(STATUS "       MSVC12")
	SET(VZPROJECT_LIBRARY_DIR
		${INTERFACE_LIBRARY_DIR}
		${PROJECT_SOURCE_DIR}/lib/vs2013
		${PROJECT_SOURCE_DIR}/src/third_part/libiconv/win32/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libcurl-7.54.0/vs2010/lib
		${PROJECT_SOURCE_DIR}/src/third_part/libevent-2.1.8/vs2010/lib
	)
	SET(VZPROJECT_LINK_LIB
		libevent_core.lib
		libvzlogging
		Iphlpapi.lib
		ws2_32.lib
		winmm.lib
	)
endif()

MESSAGE(STATUS "Step 4 : Add code source")
ADD_DEFINITIONS(
	-D_WIN32
	-DWIN32_LEAN_AND_MEAN
	-D_WIN32_WINNT=0x0502
)

#####################################################################
# Step 3 :Set visual studio runtime type
set(CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE
)
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD"  "/MT" ${CompilerFlag}  "${${CompilerFlag}}")
  string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/bin/vs2013")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/lib/vs2013")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/lib/vs2013")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/bin/vs2013")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/lib/vs2013")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/lib/vs2013")

# With Release properties
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/vs2013")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/vs2013")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/vs2013")