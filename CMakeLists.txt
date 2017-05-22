cmake_minimum_required(VERSION 3.6)
project(srcproj)

INCLUDE(cmake/global_environment_settings.cmake)
# This Include Will setting
# include path
# include library path
# link library
IF(WIN32)
    IF(MSVC12)
        INCLUDE(cmake/win_vs2013_environment_settings.cmake)
    ENDIF()
ELSEIF(UNIX)
    IF(CMAKE_BUILD_PLATFORM MATCHES ARM3730)
        INCLUDE(cmake/arm3730_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES HISI)
        INCLUDE(cmake/hisi_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES ARM8127)
        INCLUDE(cmake/arm8127_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES ARM6446)
        INCLUDE(cmake/arm6446_environment_settings.cmake)
    ENDIF()
ENDIF()

set(CMAKE_CXX_STANDARD 98)

## step 1
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(platform/imx6q)
