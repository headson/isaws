# cmake ../.. -G "Visual Studio 10 2010"
# cmake ../.. -G "Visual Studio 12 2013"
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3516 -DCMAKE_BUILD_TYPE=Release
# cmake ../.. -DCMAKE_BUILD_PLATFORM=IMX6Q -DCMAKE_BUILD_TYPE=Release
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(isaws)


INCLUDE(cmake_local/global_environment_settings.cmake)
# This Include Will setting 
# include path
# include library path
# link library
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

ADD_SUBDIRECTORY(src/lib/)
ADD_SUBDIRECTORY(src/lib/base)
ADD_SUBDIRECTORY(src/lib/shm)
ADD_SUBDIRECTORY(src/lib/vzconn)
ADD_SUBDIRECTORY(src/lib/vzlogging)
ADD_SUBDIRECTORY(src/lib/dispatcher)
#ADD_SUBDIRECTORY(src/lib/platform/imx6q)
#ADD_SUBDIRECTORY(src/lib/platform/hi3516a)


## add run
ADD_SUBDIRECTORY(src/run/vzlogserver)
#ADD_SUBDIRECTORY(src/run/hi3516a)
#ADD_SUBDIRECTORY(src/run/imx6q)
#ADD_SUBDIRECTORY(src/run/live_stream)

## add test
#ADD_SUBDIRECTORY(src/test/test_vzlogging)
<<<<<<< HEAD
ADD_SUBDIRECTORY(src/test/test_block_buffer)
ADD_SUBDIRECTORY(src/test/test_vzconn_client)
ADD_SUBDIRECTORY(src/test/test_vzconn_server)
=======
#ADD_SUBDIRECTORY(src/test/test_event)
ADD_SUBDIRECTORY(src/test/test_mcast_client)
ADD_SUBDIRECTORY(src/test/test_mcast_server)
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a

