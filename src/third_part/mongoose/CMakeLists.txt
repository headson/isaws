cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libmongoose")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "mongoose")
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
	${CMAKE_CURRENT_SOURCE_DIR}/mongoose.h
	${CMAKE_CURRENT_SOURCE_DIR}/mongoose.c
	)

SOURCE_GROUP(${BUILD_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/mongoose.h
	${CMAKE_CURRENT_SOURCE_DIR}/mongoose.c
	)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
IF (WIN32)
	ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
ELSE()
	ADD_LIBRARY(${BUILD_PROJECT_NAME} SHARED ${SRC_LIST})
ENDIF()
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_THIRD_PART})
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
)

IF(UNIX)
    add_custom_command(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libmongoose.so
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/libmongoose.so ${FTP_DIR}/libmongoose.so
	)
ENDIF()