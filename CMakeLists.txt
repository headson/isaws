# cmake ../.. -G "Visual Studio 10 2010"
# cmake ../.. -G "Visual Studio 12 2013"
# cmake ../.. -DCMAKE_BUILD_PLATFORM=HI3516 -DCMAKE_BUILD_TYPE=Release
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

ADD_SUBDIRECTORY(src/lib/base)
ADD_SUBDIRECTORY(src/lib/network)
ADD_SUBDIRECTORY(src/lib/dispatcher)

#ADD_SUBDIRECTORY(src/lib/server)
ADD_SUBDIRECTORY(src/lib/vzlogging)
#ADD_SUBDIRECTORY(src/lib/platform/imx6q)
#ADD_SUBDIRECTORY(src/lib/platform/hi3516a)


## add run
ADD_SUBDIRECTORY(src/run/vzlogserver)
#ADD_SUBDIRECTORY(src/run/hard_hi3516a)

## add test
ADD_SUBDIRECTORY(src/test/test_vzlogging)

