cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "iva_app")

#########################################################################
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib/platform/imx6q/sdk/lib/)
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 4 : Add project source")
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/iva_main.cpp
)
SOURCE_GROUP(${BUILD_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/iva_main.cpp
)
#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_EXECUTABLE(${BUILD_PROJECT_NAME} ${SRC_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})
IF(UNIX)
    ADD_CUSTOM_COMMAND(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME}
        COMMAND mv ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME} ${FTP_DIR}/${BUILD_PROJECT_NAME})
ENDIF()

#########################################################################
MESSAGE(STATUS "Step 6 : link with other library")
if(WIN32)
TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		libiva
		libdpclient
		libvzconn
		libvzbase
		libsystemv
		${VZPROJECT_LINK_LIB}
	)
else()
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
		iva
		dpclient
		vzconn
		vzbase
		systemv
		${VZPROJECT_LINK_LIB}
	)
endif()