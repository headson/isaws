cmake_minimum_required(VERSION 2.8)
#########################################################################

if(WIN32)
    SET(BUILD_PROJECT_NAME "libplatform")
elseif(UNIX)
    SET(BUILD_PROJECT_NAME "platform")
endif()

INCLUDE_DIRECTORIES(
	${YHPROJECT_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/sdk/imx6q/inc
)

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
	${CMAKE_CURRENT_SOURCE_DIR}/asc8.h
	${CMAKE_CURRENT_SOURCE_DIR}/yuv420.h
	${CMAKE_CURRENT_SOURCE_DIR}/yuv420.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/v4l2.h
	${CMAKE_CURRENT_SOURCE_DIR}/vpu.h
	${CMAKE_CURRENT_SOURCE_DIR}/vpu.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cplatform.h
	${CMAKE_CURRENT_SOURCE_DIR}/cplatform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cvideoencode.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideoencode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cimageencode.h
	${CMAKE_CURRENT_SOURCE_DIR}/cimageencode.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/cexterndevice.h
	#${CMAKE_CURRENT_SOURCE_DIR}/cexterndevice.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/calsaaudioencode.h
	#${CMAKE_CURRENT_SOURCE_DIR}/calsaaudioencode.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/calsaaudiodecode.h
	#${CMAKE_CURRENT_SOURCE_DIR}/calsaaudiodecode.cpp
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/asc8.h
	${CMAKE_CURRENT_SOURCE_DIR}/yuv420.h
	${CMAKE_CURRENT_SOURCE_DIR}/yuv420.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/v4l2.h
	${CMAKE_CURRENT_SOURCE_DIR}/vpu.h
	${CMAKE_CURRENT_SOURCE_DIR}/vpu.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cplatform.h
	${CMAKE_CURRENT_SOURCE_DIR}/cplatform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cvideoencode.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideoencode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cimageencode.h
	${CMAKE_CURRENT_SOURCE_DIR}/cimageencode.cpp
)

#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_LIBRARY(${BUILD_PROJECT_NAME} ${PROJECT_SOURCE_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})