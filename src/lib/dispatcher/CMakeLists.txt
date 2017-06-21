cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libdispatch")
	SET(BUILD_DP_CLI_NAME  "libdpclient")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "dispatch")
	SET(BUILD_DP_CLI_NAME  "dpclient")
endif()
#########################################################################
INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
#LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
SET(SRC_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.h

	${CMAKE_CURRENT_SOURCE_DIR}/server/dpsession.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/dpsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/server/dpserver.h 
	${CMAKE_CURRENT_SOURCE_DIR}/server/dpserver.cpp
)

SET(SRC_CLI_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/sync/dpclient_c.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/dpclient_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdpclient.cpp 
)

#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

ADD_LIBRARY(${BUILD_DP_CLI_NAME} STATIC ${SRC_CLI_LIST})
set_property(TARGET ${BUILD_DP_CLI_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")
