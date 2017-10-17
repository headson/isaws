cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libdevice")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "device")
endif()

#########################################################################
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libcedarc/include)
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libcedarc/library)
#LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/videodev2.h
	${CMAKE_CURRENT_SOURCE_DIR}/ccamera.h
	${CMAKE_CURRENT_SOURCE_DIR}/ccamera.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocodec.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocodec.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/view.h
	${CMAKE_CURRENT_SOURCE_DIR}/disp.h
	${CMAKE_CURRENT_SOURCE_DIR}/drv_display.h

	${CMAKE_CURRENT_SOURCE_DIR}/cvideocapture.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocapture.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.h
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.cpp
)

SOURCE_GROUP(camera FILES
	${CMAKE_CURRENT_SOURCE_DIR}/videodev2.h
	${CMAKE_CURRENT_SOURCE_DIR}/ccamera.h
	${CMAKE_CURRENT_SOURCE_DIR}/ccamera.cpp
)

SOURCE_GROUP(encodec FILES
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocodec.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocodec.cpp
)

SOURCE_GROUP(display FILES
	${CMAKE_CURRENT_SOURCE_DIR}/view.h
	${CMAKE_CURRENT_SOURCE_DIR}/disp.h
	${CMAKE_CURRENT_SOURCE_DIR}/drv_display.h
)

SOURCE_GROUP(listen FILES
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocapture.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideocapture.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.h
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.cpp
)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
	
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")
TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
)