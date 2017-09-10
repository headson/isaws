cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
if(WIN32)
	SET(BUILD_PROJECT_NAME_LOGGING		"libvzlogging")
	SET(BUILD_PROJECT_NAME_LOGSERVER	"libvzlogserver")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME_LOGGING		"vzlogging")
	SET(BUILD_PROJECT_NAME_LOGSERVER	"vzlogserver")
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

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}../)
#########################################################################
#Step 3 : Add code source
SET(SRC_LIST_BASE
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzbases.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzbases.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzcommon.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzcommon.cpp
)

SET(SRC_LIST_LOGGING
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzlogging.h
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzlogging.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzloggingcpp.h
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzloggingcpp.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzwatchdog.h
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzwatchdog.cpp
)

SET(SRC_LIST_LOGSERVER
	${CMAKE_CURRENT_SOURCE_DIR}/server/getopt.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/getopt.c
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogmanage.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogmanage.cpp
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzbases.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzbases.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzcommon.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vzcommon.cpp
)

SOURCE_GROUP(logging FILES
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzlogging.h
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzlogging.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzloggingcpp.h
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzloggingcpp.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzwatchdog.h
	${CMAKE_CURRENT_SOURCE_DIR}/logging/vzwatchdog.cpp
)

SOURCE_GROUP(server FILES
	${CMAKE_CURRENT_SOURCE_DIR}/server/getopt.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/getopt.c
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogmanage.h
	${CMAKE_CURRENT_SOURCE_DIR}/server/cvzlogmanage.cpp
)
## build library vzlogging
MESSAGE(STATUS "Build vzlogserver library")
add_library(${BUILD_PROJECT_NAME_LOGSERVER} STATIC
        ${SRC_LIST_BASE}
        ${SRC_LIST_LOGSERVER})
set_property(TARGET ${BUILD_PROJECT_NAME_LOGSERVER} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

MESSAGE(STATUS "Build vzlogging library")
if(WIN32)
	add_library(${BUILD_PROJECT_NAME_LOGGING} STATIC
			${SRC_LIST_BASE}
			${SRC_LIST_LOGGING})

	set_property(TARGET ${BUILD_PROJECT_NAME_LOGGING} 
		PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
	
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME_LOGGING} 
		ws2_32.lib
	)
else()
	add_library(${BUILD_PROJECT_NAME_LOGGING} SHARED
			${SRC_LIST_BASE}
			${SRC_LIST_LOGGING})

	set_property(TARGET ${BUILD_PROJECT_NAME_LOGGING} 
		PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME_LOGGING} 
		#libevent_core.so
		#libevent_pthreads.so
	)
endif()

IF(UNIX)
    add_custom_command(TARGET ${BUILD_PROJECT_NAME_LOGGING} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libvzlogging.so
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libvzlogging.so ${FTP_DIR}/libvzlogging.so
		)
ENDIF()