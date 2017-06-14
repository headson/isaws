# cmake ../.. -G "Visual Studio 10 2010"
# cmake ../.. -G "Visual Studio 12 2013"
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3516 -DCMAKE_BUILD_TYPE=Release
# cmake ../.. -DCMAKE_BUILD_PLATFORM=IMX6Q -DCMAKE_BUILD_TYPE=Release
cmake_minimum_required(VERSION 3.6)
project(isaw_project)

INCLUDE(cmake_local/global_environment_settings.cmake)
IF(WIN32)
    IF(MSVC12)
        INCLUDE(cmake_local/win_vs2013_environment_settings.cmake)
    ELSEIF(MSVC10)
        INCLUDE(cmake_local/win_vs2010_environment_settings.cmake)
    ENDIF()
ELSEIF(UNIX)
    IF(CMAKE_BUILD_PLATFORM MATCHES IMX6Q)
        INCLUDE(cmake_local/arm_imx6q_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES HI3516)
        INCLUDE(cmake_local/arm_hi3516_environment_settings.cmake)
    ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(${VZPROJECT_INCLUDE_DIR})
LINK_DIRECTORIES(${VZPROJECT_LIBRARY_DIR})

## add lib
ADD_SUBDIRECTORY(cmake_local)

## add third sdk
#ADD_SUBDIRECTORY(src/third_sdk/live555)

ADD_SUBDIRECTORY(src/lib/base)
ADD_SUBDIRECTORY(src/lib/shm)
ADD_SUBDIRECTORY(src/lib/vzlogging)
ADD_SUBDIRECTORY(src/lib/network)
#ADD_SUBDIRECTORY(src/lib/dispatcher)
#ADD_SUBDIRECTORY(src/lib/platform/imx6q)
#ADD_SUBDIRECTORY(src/lib/platform/hi3516a)


## add run
ADD_SUBDIRECTORY(src/run/watchdog)
#ADD_SUBDIRECTORY(src/run/hi3516a)
#ADD_SUBDIRECTORY(src/run/imx6q)
#ADD_SUBDIRECTORY(src/run/live_stream)

## add test
#ADD_SUBDIRECTORY(src/test/test_vzlogging)
#ADD_SUBDIRECTORY(src/test/test_event)
ADD_SUBDIRECTORY(src/test/test_mcast_client)
ADD_SUBDIRECTORY(src/test/test_mcast_server)

