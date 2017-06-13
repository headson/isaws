cmake_minimum_required(VERSION 2.8)
#########################################################################

if(WIN32)
	SET(BUILD_PROJECT_NAME "libbase")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "base")
endif()

#########################################################################
MESSAGE(STATUS "Print the include directores")
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
MESSAGE(STATUS "Step 3 : Add code source")
SET(SRC_LIST
	#${CMAKE_CURRENT_SOURCE_DIR}/base/helper.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/base/helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/flags/yhflags.h
	${CMAKE_CURRENT_SOURCE_DIR}/flags/getopt.h
	${CMAKE_CURRENT_SOURCE_DIR}/flags/getopt.c
	
	${CMAKE_CURRENT_SOURCE_DIR}/vtypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h
	${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/verror.h
	${CMAKE_CURRENT_SOURCE_DIR}/verror.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vmessage.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/core/vtls.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vtls.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/core/vlock.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vlock.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/core/vthread.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vthread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/core/vdatetime.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vdatetime.cpp
)

SOURCE_GROUP(base FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/base/helper.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/base/helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/aes.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/base/authmethods.h
)
SOURCE_GROUP(flags FILES
	${CMAKE_CURRENT_SOURCE_DIR}/flags/yhflags.h
	${CMAKE_CURRENT_SOURCE_DIR}/flags/getopt.h
	${CMAKE_CURRENT_SOURCE_DIR}/flags/getopt.c
)
SOURCE_GROUP(inc FILES
	${CMAKE_CURRENT_SOURCE_DIR}/vtypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h
	${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/verror.h
	${CMAKE_CURRENT_SOURCE_DIR}/verror.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vmessage.h
)
SOURCE_GROUP(core FILES
	${CMAKE_CURRENT_SOURCE_DIR}/core/vtls.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vtls.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/core/vlock.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vlock.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/core/vthread.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vthread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/core/vdatetime.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/vdatetime.cpp
)
#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB}
)
