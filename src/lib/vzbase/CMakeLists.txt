cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libvzbase")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "vzbase")
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

IF(WIN32)
	SET(WIN32_LOAD_FILE
		${CMAKE_CURRENT_SOURCE_DIR}/base/win32.h
		${CMAKE_CURRENT_SOURCE_DIR}/base/win32.cpp
	)
ELSEIF()
	SET(WIN32_LOAD_FILE
	)
ENDIF()

SET(SRC_LIST 	
	${CMAKE_CURRENT_SOURCE_DIR}/base/basictypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/boost_settings.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/criticalsection.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/constructormagic.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/noncoypable.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.h
	${WIN32_LOAD_FILE}
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/vmessage.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/system/vtls.h
	${CMAKE_CURRENT_SOURCE_DIR}/system/vtls.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/system/vshm.h
	${CMAKE_CURRENT_SOURCE_DIR}/system/vshm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/system/mysystem.h

	${CMAKE_CURRENT_SOURCE_DIR}/helper/stdafx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stdafx.h
	#${CMAKE_CURRENT_SOURCE_DIR}/helper/getopt.c
	#${CMAKE_CURRENT_SOURCE_DIR}/helper/getopt.h
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stringhelper.h
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stringhelper.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/thread/thread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread/thread.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagequeue.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagequeue.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagehandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagehandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread/eventserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/eventserver.cpp
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/basictypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/boost_settings.hpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/helper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/criticalsection.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/constructormagic.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/noncoypable.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/timeutils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.h
	${WIN32_LOAD_FILE}
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/vmessage.h
)

SOURCE_GROUP(helper FILES
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stdafx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stdafx.h
	#${CMAKE_CURRENT_SOURCE_DIR}/helper/getopt.c
	#${CMAKE_CURRENT_SOURCE_DIR}/helper/getopt.h
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stringhelper.h
	${CMAKE_CURRENT_SOURCE_DIR}/helper/stringhelper.cpp
)

SOURCE_GROUP(system FILES
	${CMAKE_CURRENT_SOURCE_DIR}/system/vtls.h
	${CMAKE_CURRENT_SOURCE_DIR}/system/vtls.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/system/vshm.h
	${CMAKE_CURRENT_SOURCE_DIR}/system/vshm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/system/mysystem.h
)

SOURCE_GROUP(thread FILES
	${CMAKE_CURRENT_SOURCE_DIR}/thread/thread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread/thread.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagequeue.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagequeue.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagehandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/messagehandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread/eventserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/thread/eventserver.cpp
)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
if(WIN32)
	ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
ELSE()
	ADD_LIBRARY(${BUILD_PROJECT_NAME} SHARED ${SRC_LIST})
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		vzlogging
		libevent_core.so
		libevent_pthreads.so
	)
    add_custom_command(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libvzbase.so
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libvzbase.so ${FTP_DIR}/libvzbase.so
	)
ENDIF()
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")
