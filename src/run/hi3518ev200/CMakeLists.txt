cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "platform_app")
SET(BUILD_CLOCK_PROJECT_NAME "clock")

#########################################################################
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib/platform/hi3518ev200/include)
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib/platform/hi3518ev200/lib/)
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 4 : Add project source")
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518e_main.cpp
)
SOURCE_GROUP(${BUILD_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518e_main.cpp
)

SET(CLK_SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/pcf8563.cpp
)
SOURCE_GROUP(${BUILD_CLOCK_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/pcf8563.cpp
)

#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")
ADD_DEFINITIONS(
	-Dhi3518ev200
	-DHICHIP=0x3518E200
	-DSENSOR_TYPE=APTINA_AR0130_DC_720P_30FPS
	-DCHIP_ID=CHIP_HI3518E_V200
	-DHI_DEBUG 
	-DHI_XXXX 
	-DISP_V2
)

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_EXECUTABLE(${BUILD_PROJECT_NAME} ${SRC_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})
IF(UNIX)
    ADD_CUSTOM_COMMAND(TARGET ${BUILD_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME}
        COMMAND mv ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_PROJECT_NAME} ${FTP_DIR}/${BUILD_PROJECT_NAME})
ENDIF()

ADD_EXECUTABLE(${BUILD_CLOCK_PROJECT_NAME} ${CLK_SRC_LIST})
SET_PROPERTY(TARGET ${BUILD_CLOCK_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})
IF(UNIX)
    ADD_CUSTOM_COMMAND(TARGET ${BUILD_CLOCK_PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_CLOCK_PROJECT_NAME}
        COMMAND mv ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BUILD_CLOCK_PROJECT_NAME} ${FTP_DIR}/${BUILD_CLOCK_PROJECT_NAME})
ENDIF()

#########################################################################
MESSAGE(STATUS "Step 6 : link with other library")
if(WIN32)
else()
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME}
		libcount.so
		platform
		systemv
		dpclient
		vzconn
		vzbase
		jsoncpp
		libmpi.a 
		libive.a
		libmd.a 
		libtde.a 
		libVoiceEngine.a
		libupvqe.a 
		libdnvqe.a
		libisp.a
		libsns_ar0130.a
		lib_cmoscfg.a
		lib_iniparser.a
		lib_hiae.a 
		lib_hiawb.a 
		lib_hiaf.a
		lib_hidefog.a
		${VZPROJECT_LINK_LIB}
	)
endif()