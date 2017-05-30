cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "vzlogserverapp")

#########################################################################
MESSAGE(STATUS "Step 1 : Add include directories")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 2 : Add library directories")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 3 : Add code source")
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/vzlogserver_main.cpp
)

#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
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
	ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		${VZPROJECT_LINK_LIB}
		libbase.lib
		libvzlogserver.lib
	)
else()
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
        ${VZPROJECT_LINK_LIB}
		libbase.a
		libvzlogserver.a
	)
endif()