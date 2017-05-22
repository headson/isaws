cmake_minimum_required(VERSION 3.6)
project(srcproj)

INCLUDE(cmake_local/global_environment_settings.cmake)
IF(WIN32)
    IF(MSVC12)
        INCLUDE(cmake_local/win_vs2013_environment_settings.cmake)
    ENDIF()
ELSEIF(UNIX)
    IF(CMAKE_BUILD_PLATFORM MATCHES ARM3730)
        INCLUDE(cmake_local/armimx6q_environment_settings.cmake)
    ENDIF()
ENDIF()

SET(CMAKE_CXX_STANDARD 98)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/lib)

## step 1
ADD_SUBDIRECTORY(cmake_local)

ADD_SUBDIRECTORY(src/lib/base)
ADD_SUBDIRECTORY(src/lib/server)


ADD_SUBDIRECTORY(src/lib/platform/imx6q)
