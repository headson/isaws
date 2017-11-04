cmake_minimum_required(VERSION 2.8)

if(WIN32)
    SET(BUILD_PROJECT_NAME "libclient_hs")
elseif(UNIX)
    SET(BUILD_PROJECT_NAME "client_hs")
endif()

#########################################################################
MESSAGE(STATUS "Step 1 : Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 3 : Add project code")
SET(PROJECT_SOURCE_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/cmarkup.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/cmarkup.cpp 
	
	${CMAKE_CURRENT_SOURCE_DIR}/client/ctcpasyncclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/ctcpasyncclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientaccess.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientaccess.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientdispatch.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientdispatch.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientstorage.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientstorage.cpp 
	
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.h 
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.cpp
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/base/basedefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/cmarkup.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/cmarkup.cpp	
)

SOURCE_GROUP(client FILES
	${CMAKE_CURRENT_SOURCE_DIR}/client/ctcpasyncclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/ctcpasyncclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientaccess.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientaccess.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientdispatch.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientdispatch.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientstorage.h
	${CMAKE_CURRENT_SOURCE_DIR}/client/cclientstorage.cpp 
)

SOURCE_GROUP(message FILES
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.h
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.cpp
)

#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_LIBRARY(${BUILD_PROJECT_NAME} ${PROJECT_SOURCE_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})