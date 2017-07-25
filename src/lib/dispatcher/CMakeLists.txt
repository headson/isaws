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

	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpsession.h
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpserver.h 
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpserver.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbsqlite.h 
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbsqlite.cpp
)

SET(SRC_CLI_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/sync/dpclient_c.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/dpclient_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdpclient.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdppollclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdppollclient.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ckvdbclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ckvdbclient.cpp 
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.h
)

SOURCE_GROUP(dpserver FILES
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpsession.h
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpserver.h 
	${CMAKE_CURRENT_SOURCE_DIR}/dpserver/dpserver.cpp
)


SOURCE_GROUP(kvdbserver FILES
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbsqlite.h 
	${CMAKE_CURRENT_SOURCE_DIR}/kvdbserver/kvdbsqlite.cpp
)

SOURCE_GROUP(client FILES
	${CMAKE_CURRENT_SOURCE_DIR}/sync/dpclient_c.h 
	${CMAKE_CURRENT_SOURCE_DIR}/sync/dpclient_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdpclient.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdppollclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/cdppollclient.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ckvdbclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ckvdbclient.cpp 
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

IF(WIN32)
	ADD_LIBRARY(${BUILD_DP_CLI_NAME} STATIC ${SRC_CLI_LIST})
ELSE()
	ADD_LIBRARY(${BUILD_DP_CLI_NAME} SHARED ${SRC_CLI_LIST})
ENDIF()

set_property(TARGET ${BUILD_DP_CLI_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

if(WIN32)
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
	)
else()
	TARGET_LINK_LIBRARIES(${BUILD_DP_CLI_NAME} 
		vzconn
		vzbase
		vzlogging
		libevent_core.so
		libevent_pthreads.so
	)
    add_custom_command(TARGET ${BUILD_DP_CLI_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libdpclient.so
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libdpclient.so ${FTP_DIR}/libdpclient.so
	)
endif()