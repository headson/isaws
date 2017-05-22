cmake_minimum_required(VERSION 2.8)

if(WIN32)
    SET(LIB_SERVER_NAME "libserver")
elseif(UNIX)
    SET(LIB_SERVER_NAME "server")
endif()

INCLUDE_DIRECTORIES(
	${ISAW_INCLUDE_PATH}
	${PROJECT_SOURCE_DIR}/src/lib/
)

SET(SERVER_SRC_LIST
        csockdgram.h
		csockdgram.cpp
		csockstream.h
		csockstream.cpp)

ADD_LIBRARY(${LIB_SERVER_NAME}
        ${SERVER_SRC_LIST})
