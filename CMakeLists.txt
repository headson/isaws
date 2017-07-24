# cmake ../.. -G "Visual Studio 10 2010"
# cmake ../.. -G "Visual Studio 12 2013"
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3516 -DCMAKE_BUILD_TYPE=Release
# cmake ../.. -DCMAKE_BUILD_PLATFORM=IMX6Q -DCMAKE_BUILD_TYPE=Release
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3518 -DCMAKE_BUILD_TYPE=Release
cmake_minimum_required(VERSION 3.6)
project(isaw_project)
INCLUDE(cmake/global_environment_settings.cmake)

IF(WIN32)
    IF(MSVC12)
        INCLUDE(cmake/win_vs2013_environment_settings.cmake)
    ELSEIF(MSVC10)
        INCLUDE(cmake/win_vs2010_environment_settings.cmake)
    ENDIF()
ELSEIF(UNIX)
    IF(CMAKE_BUILD_PLATFORM MATCHES IMX6Q)
        INCLUDE(cmake/arm_imx6q_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES HI3516)
        INCLUDE(cmake/arm_hi3516_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES HI3518)
        INCLUDE(cmake/arm_hi3518_environment_settings.cmake)
    ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(${VZPROJECT_INCLUDE_DIR})
LINK_DIRECTORIES(${VZPROJECT_LIBRARY_DIR})

## add lib
ADD_SUBDIRECTORY(cmake)

## add third sdk
ADD_SUBDIRECTORY(src/third_part/json)
#ADD_SUBDIRECTORY(src/third_part/live555)
ADD_SUBDIRECTORY(src/third_part/sqlite3)

ADD_SUBDIRECTORY(src/lib/vzbase)
ADD_SUBDIRECTORY(src/lib/vzlogging)
ADD_SUBDIRECTORY(src/lib/web_server)
ADD_SUBDIRECTORY(src/lib/vzconn)
ADD_SUBDIRECTORY(src/lib/systemv)
#ADD_SUBDIRECTORY(src/lib/onvif)
ADD_SUBDIRECTORY(src/lib/systemserver)
ADD_SUBDIRECTORY(src/lib/dispatcher)
#ADD_SUBDIRECTORY(src/lib/platform/hi3518ev200)

## add run
#ADD_SUBDIRECTORY(src/run/onvif)
ADD_SUBDIRECTORY(src/run/web_server)
ADD_SUBDIRECTORY(src/run/vzlogserver)
ADD_SUBDIRECTORY(src/run/system_server)
ADD_SUBDIRECTORY(src/run/dispatcherserver)
#ADD_SUBDIRECTORY(src/run/hi3518ev200)
#ADD_SUBDIRECTORY(src/run/live_stream)

IF(WIN32)
#ADD_SUBDIRECTORY(src/run/pc_manage)
ENDIF()
ADD_SUBDIRECTORY(src/web)

## add test
#ADD_SUBDIRECTORY(src/test/test_systemv)
#ADD_SUBDIRECTORY(src/test/test_vzlogging)
#ADD_SUBDIRECTORY(src/test/test_h264_to_flv)
#ADD_SUBDIRECTORY(src/test/test_dispatch/test_dp_client)
#ADD_SUBDIRECTORY(src/test/test_dispatch/test_dp_worker)
#ADD_SUBDIRECTORY(src/test/test_dispatch/test_kvdb_client)
#ADD_SUBDIRECTORY(src/test/test_multicast)
#ADD_SUBDIRECTORY(src/test/test_curl/test_curl_event)
#ADD_SUBDIRECTORY(src/test/test_vzconn/test_vzconn_client)
#ADD_SUBDIRECTORY(src/test/test_vzconn/test_vzconn_server)

