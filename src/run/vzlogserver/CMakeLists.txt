cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "watchdog")
SET(BUILD_FEEDDOG_NAME "feeddog")

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
SET(SRC_HISI_DOG_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/hisi_dog/vzhardwaredog.h
	${CMAKE_CURRENT_SOURCE_DIR}/hisi_dog/vzhardwaredog.cpp
)

SET(SRC_WATCHDOG_LIST 
	${SRC_HISI_DOG_LIST}
	${CMAKE_CURRENT_SOURCE_DIR}/vzlogserver_main.cpp
)
SOURCE_GROUP(${BUILD_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/vzlogserver_main.cpp
)

SET(SRC_FEEDDOG_LIST 	
	${CMAKE_CURRENT_SOURCE_DIR}/feeddog_main.cpp
)
SOURCE_GROUP(${BUILD_FEEDDOG_NAME} FILES
	${SRC_HISI_DOG_LIST}
	${CMAKE_CURRENT_SOURCE_DIR}/feeddog_main.cpp
)

SOURCE_GROUP(HISI_DOG FILES
	${SRC_HISI_DOG_LIST}
)
#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
# WATCHDOG
ADD_executable(${BUILD_PROJECT_NAME} ${SRC_WATCHDOG_LIST} ${SRC_HISI_DOG_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})
IF(UNIX)
    add_custom_command(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME}
        COMMAND cp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME} ${FTP_DIR}/${BUILD_PROJECT_NAME})
ENDIF()

# FEEDDOG
ADD_executable(${BUILD_FEEDDOG_NAME} ${SRC_FEEDDOG_LIST} ${SRC_HISI_DOG_LIST})
set_property(TARGET ${BUILD_FEEDDOG_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})
IF(UNIX)
	add_custom_command(TARGET ${BUILD_FEEDDOG_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_FEEDDOG_NAME}
        COMMAND cp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_FEEDDOG_NAME} ${FTP_DIR}/${BUILD_FEEDDOG_NAME})
ENDIF()
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")
if(WIN32)
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		libvzlogserver
		ws2_32.lib
	)
	
	TARGET_LINK_LIBRARIES(${BUILD_FEEDDOG_NAME} 
		libvzlogging
		ws2_32.lib
	)
else()
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME}
		vzlogserver
		pthread
		rt
		-Wl,-rpath=.
	)
	
	TARGET_LINK_LIBRARIES(${BUILD_FEEDDOG_NAME}
		vzlogging
		pthread
		rt
		-Wl,-rpath=.
	)
endif()