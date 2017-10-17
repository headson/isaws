cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libvzconn")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "vzconn")
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
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h  
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/clibevent.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/clibevent.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/cblockbuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/cblockbuffer.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/byteorder.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/bytebuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/bytebuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/perfectbufferpool.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/perfectbufferpool.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/multicast/cmcastsocket.h
	${CMAKE_CURRENT_SOURCE_DIR}/multicast/cmcastsocket.cpp
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefines.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/base/clibevent.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/clibevent.cpp
)

SOURCE_GROUP(buffer FILES
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/cblockbuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/cblockbuffer.cpp 
	
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/byteorder.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/bytebuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/bytebuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/perfectbufferpool.h
	${CMAKE_CURRENT_SOURCE_DIR}/buffer/perfectbufferpool.cpp
)

SOURCE_GROUP(async FILES
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.cpp
)

SOURCE_GROUP(sync FILES
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.cpp
)

SOURCE_GROUP(multicast FILES
	${CMAKE_CURRENT_SOURCE_DIR}/multicast/cmcastsocket.h
	${CMAKE_CURRENT_SOURCE_DIR}/multicast/cmcastsocket.cpp
)

#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})

#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

if(WIN32)
	ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
	)
else()
	ADD_LIBRARY(${BUILD_PROJECT_NAME} SHARED ${SRC_LIST})
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		vzbase
		vzlogging
		libevent_core.so
		libevent_pthreads.so
	)
endif()
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

IF(UNIX)
    add_custom_command(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libvzconn.so
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libvzconn.so ${FTP_DIR}/libvzconn.so
		)
ENDIF()