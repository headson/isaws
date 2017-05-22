cmake_minimum_required(VERSION 2.8)

if(WIN32)
    SET(LIB_PLATFORM_NAME "libplatform")
elseif(UNIX)
    SET(LIB_PLATFORM_NAME "platform")
endif()

INCLUDE_DIRECTORIES(
	${ISAW_INCLUDE_PATH}
	${PROJECT_SOURCE_DIR}/src/inc
	${PROJECT_SOURCE_DIR}/sdk/imx6q/inc
)

SET(PLATFORM_SRC_LIST
	asc8.h
	yuv420.h
	yuv420.cpp
	v4l2.h
	vpu.h
	vpu.cpp
	cplatform.h
	cplatform.cpp
	cvideoencode.h
	cvideoencode.cpp
	cphotoencode.h
	cphotoencode.cpp
	cexterndevice.h
	cexterndevice.cpp
	calsaaudioencode.h
	calsaaudioencode.cpp
	calsaaudiodecode.h
	calsaaudiodecode.cpp
)

ADD_LIBRARY(${LIB_PLATFORM_NAME}
        ${PLATFORM_SRC_LIST})