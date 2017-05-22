cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 98)

if(WIN32)
    SET(LIB_BASE_NAME "libbase")
elseif(UNIX)
    SET(LIB_BASE_NAME "base")
endif()

set(LIB_BASE_SRC_LIST
        vshm.h
        vshm.cpp
        cshmvideo.h
        cshmvideo.cpp
        cshmimage.h
        cshmimage.cpp)

add_library(${LIB_BASE_NAME}
        ${LIB_BASE_SRC_LIST})
