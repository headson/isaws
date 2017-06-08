cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "live_stream")

#########################################################################
INCLUDE_DIRECTORIES(
	C:/tools/sdk/opencv-2.4.13/build/install/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/src/third_sdk/
	${PROJECT_SOURCE_DIR}/src/third_sdk/live555/groupsock/include
	${PROJECT_SOURCE_DIR}/src/third_sdk/live555/liveMedia/include
	${PROJECT_SOURCE_DIR}/src/third_sdk/live555/UsageEnvironment/include
	${PROJECT_SOURCE_DIR}/src/third_sdk/live555/BasicUsageEnvironment/include
)
MESSAGE(STATUS "Print the include directores")
LINK_DIRECTORIES(C:/tools/sdk/opencv-2.4.13/build/install/x86/vc10/lib)

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
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/clivestream.h
	${CMAKE_CURRENT_SOURCE_DIR}/clivestream.cpp
)
SOURCE_GROUP(${BUILD_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/clivestream.h
	${CMAKE_CURRENT_SOURCE_DIR}/clivestream.cpp
)
#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})
IF(UNIX)
    add_custom_command(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME}
        COMMAND cp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME} ${FTP_DIR}/${BUILD_PROJECT_NAME})
ENDIF()

#########################################################################
MESSAGE(STATUS "Step 6 : link with other library")
if(WIN32)
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		libvzlogging.lib
		BasicUsageEnvironment
		groupsock
		liveMedia
		UsageEnvironment
		ws2_32
		winmm
	)
else()
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		libvzlogging.a
		BasicUsageEnvironment
		groupsock
		liveMedia
		UsageEnvironment
		pthread
		rt
	)
endif()