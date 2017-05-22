cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 98)

if(WIN32)
    SET(LIB_COMMON_NAME "libcommon")
elseif(UNIX)
    SET(LIB_COMMON_NAME "common")
endif()

set(LIB_COMMON_SRC_LIST
        vshm.h
        vshm.cpp
        cshmvideo.h
        cshmvideo.cpp)

add_library(${LIB_COMMON_NAME}
        ${LIB_COMMON_SRC_LIST})