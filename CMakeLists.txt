cmake_minimum_required(VERSION 3.6)
project(srcproj)

INCLUDE(cmake_local/global_environment_settings.cmake)
IF(WIN32)
    IF(MSVC12)
        INCLUDE(cmake_local/win_vs2013_environment_settings.cmake)
	ELSEIF(MSVC10)
		INCLUDE(cmake_local/win_vs2010_environment_settings.cmake)
    ENDIF()
ELSEIF(UNIX)
    IF(CMAKE_BUILD_PLATFORM MATCHES ARM3730)
        INCLUDE(cmake_local/armimx6q_environment_settings.cmake)
    ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(${VZPROJECT_INCLUDE_DIR})
LINK_DIRECTORIES(${VZPROJECT_LIBRARY_DIR})

## add lib
ADD_SUBDIRECTORY(cmake_local)

ADD_SUBDIRECTORY(src/lib/base)
#ADD_SUBDIRECTORY(src/lib/network)

ADD_SUBDIRECTORY(src/lib/dispatcher/client)
ADD_SUBDIRECTORY(src/lib/dispatcher/server)
ADD_SUBDIRECTORY(src/lib/dispatcher/worker)

#ADD_SUBDIRECTORY(src/lib/server)
ADD_SUBDIRECTORY(src/lib/vzlogging)
#ADD_SUBDIRECTORY(src/lib/platform/imx6q)

## add run
ADD_SUBDIRECTORY(src/run/vzlogserver)

## add test
ADD_SUBDIRECTORY(src/test/test_vzlogging)

