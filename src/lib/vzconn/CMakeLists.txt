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
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/cblockbuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/cblockbuffer.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/clibevent.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/clibevent.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.cpp
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/pkghead.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/vsocket.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/cblockbuffer.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/cblockbuffer.cpp
)

SOURCE_GROUP(sync FILES
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/sync/ctcpclient.cpp
)

SOURCE_GROUP(async FILES
	${CMAKE_CURRENT_SOURCE_DIR}/async/clibevent.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/clibevent.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevttcpserver.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.h
	${CMAKE_CURRENT_SOURCE_DIR}/async/cevtipcserver.cpp
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

#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")