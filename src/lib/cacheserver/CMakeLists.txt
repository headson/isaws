cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libcachedserver")
	
	SET(BUILD_CACHE_CLI_NAME "libcachedclient")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "cachedserver")
	
	SET(BUILD_CACHE_CLI_NAME "cachedclient")
endif()

#########################################################################
#INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
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
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedserver.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedstanzapool.h
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedstanzapool.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedsession.h
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedservice.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedservice.cpp
	)
	
SET(SRC_CLI_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient_c.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient_c.cpp
	)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h
	)

SOURCE_GROUP(server FILES
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedserver.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedstanzapool.h
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedstanzapool.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedsession.h
	#${CMAKE_CURRENT_SOURCE_DIR}/server/cachedsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedservice.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cachedservice.cpp
	)

SOURCE_GROUP(client FILES
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient_c.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cachedclient_c.cpp
	)

#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
if(WIN32)
	##ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
	ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
ELSE()
	ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
ENDIF()
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

ADD_LIBRARY(${BUILD_CACHE_CLI_NAME} STATIC ${SRC_CLI_LIST})
set_property(TARGET ${BUILD_CACHE_CLI_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
#IF(UNIX)
#    add_custom_command(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
#        COMMAND ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libcachedserver.so
#        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libcachedserver.so ${FTP_DIR}/libcachedserver.so)
#ENDIF()
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

if(WIN32)
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
	)
else()
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
	)
endif()