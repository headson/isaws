cmake_minimum_required(VERSION 2.8)

if(WIN32)
    SET(BUILD_PROJECT_NAME "libsystemv")
elseif(UNIX)
    SET(BUILD_PROJECT_NAME "systemv")
endif()

#########################################################################
MESSAGE(STATUS "Step 1 : Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 3 : Add project code")
SET(PROJECT_SOURCE_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/flvmux/cflvmux.h
	${CMAKE_CURRENT_SOURCE_DIR}/flvmux/cflvmux.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/shm/vzshm_c.h
	${CMAKE_CURRENT_SOURCE_DIR}/shm/vzshm_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shm/vz_shm.h
	${CMAKE_CURRENT_SOURCE_DIR}/shm/vz_shm.cpp
)

SOURCE_GROUP(flvmux FILES
	${CMAKE_CURRENT_SOURCE_DIR}/flvmux/cflvmux.h
	${CMAKE_CURRENT_SOURCE_DIR}/flvmux/cflvmux.cpp
)

SOURCE_GROUP(shm FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/shm/vzshm_c.h
	${CMAKE_CURRENT_SOURCE_DIR}/shm/vzshm_c.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shm/vz_shm.h
	${CMAKE_CURRENT_SOURCE_DIR}/shm/vz_shm.cpp
)
#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${PROJECT_SOURCE_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})