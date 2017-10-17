# cmake ../.. -G "Visual Studio 10 2010"
# cmake ../.. -G "Visual Studio 12 2013"
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3516 -DCMAKE_BUILD_TYPE=Release
# cmake ../.. -DCMAKE_BUILD_PLATFORM=IMX6Q -DCMAKE_BUILD_TYPE=Release
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3518 -DCMAKE_BUILD_TYPE=Release
cmake_minimum_required(VERSION 2.8)
project(isaw_project)
INCLUDE(cmake/global_environment_settings.cmake)

IF(WIN32)
    IF(MSVC12)
        INCLUDE(cmake/win_vs2013_environment_settings.cmake)
    ELSEIF(MSVC10)
        INCLUDE(cmake/win_vs2010_environment_settings.cmake)
    ENDIF()
ELSEIF(UNIX)
    #ADD_COMPILE_OPTIONS(-fpermissive)
    IF(CMAKE_BUILD_PLATFORM MATCHES LINUX)
        INCLUDE(cmake/linux_u1004_environment_settings.cmake)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES IMX6Q)
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
ADD_SUBDIRECTORY(src/third_part/mongoose)

ADD_SUBDIRECTORY(src/lib/vzbase)
ADD_SUBDIRECTORY(src/lib/vzlogging)
ADD_SUBDIRECTORY(src/lib/vzconn)
ADD_SUBDIRECTORY(src/lib/systemv)
#ADD_SUBDIRECTORY(src/lib/onvif)
ADD_SUBDIRECTORY(src/lib/web_server)
ADD_SUBDIRECTORY(src/lib/systemserver)
ADD_SUBDIRECTORY(src/lib/dispatcher)
ADD_SUBDIRECTORY(src/lib/dev_sunxi_v40)
#ADD_SUBDIRECTORY(src/lib/platform)
#ADD_SUBDIRECTORY(src/lib/platform/imx6q)
ADD_SUBDIRECTORY(src/lib/http_sender)
ADD_SUBDIRECTORY(src/lib/business)
#ADD_SUBDIRECTORY(src/lib/web_for_dp)

## add run
#ADD_SUBDIRECTORY(src/run/onvif)
ADD_SUBDIRECTORY(src/run/web_server)
ADD_SUBDIRECTORY(src/run/feeddog)
ADD_SUBDIRECTORY(src/run/vzlogserver)
ADD_SUBDIRECTORY(src/run/system_server)
ADD_SUBDIRECTORY(src/run/dispatcherserver)
#ADD_SUBDIRECTORY(src/run/hi3518ev200)
#ADD_SUBDIRECTORY(src/run/imx6q)
ADD_SUBDIRECTORY(src/run/pack_image)
ADD_SUBDIRECTORY(src/run/business)
#ADD_SUBDIRECTORY(src/run/fix_sqlite)
ADD_SUBDIRECTORY(src/run/http_sender)
#ADD_SUBDIRECTORY(src/run/live_stream)
#ADD_SUBDIRECTORY(src/run/web_for_dp)

IF(WIN32)
ADD_SUBDIRECTORY(src/lib/curlclient)

ADD_SUBDIRECTORY(src/run/icamera)
ADD_SUBDIRECTORY(src/run/pc_manage)

ADD_SUBDIRECTORY(src/web)

ADD_SUBDIRECTORY(src/third_part/googletest)

## google test 
#ADD_SUBDIRECTORY(src/gtest/gtest_vzbase)
#ADD_SUBDIRECTORY(src/gtest/gtest_vzconn)
ENDIF()

## add test
#ADD_SUBDIRECTORY(src/test/test_systemv)
#ADD_SUBDIRECTORY(src/test/test_vzlogging)
#ADD_SUBDIRECTORY(src/test/test_h264_to_flv)
ADD_SUBDIRECTORY(src/test/test_dispatch/test_dp_client)
ADD_SUBDIRECTORY(src/test/test_dispatch/test_dp_worker)
#ADD_SUBDIRECTORY(src/test/test_multicast)
#ADD_SUBDIRECTORY(src/test/test_curl/test_curl_event)
#ADD_SUBDIRECTORY(src/test/test_sqlite/restore)
#ADD_SUBDIRECTORY(src/test/test_vzconn/test_vzconn_client)
#ADD_SUBDIRECTORY(src/test/test_vzconn/test_vzconn_server)
#ADD_SUBDIRECTORY(src/test/test_catch_event)
#ADD_SUBDIRECTORY(src/test/test_ascii_to_bmp)
