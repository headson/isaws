cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "icamera")

#########################################################################
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/run/icamera/camera/directshow/include)
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/run/icamera)
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/icamrea_main.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/ch264code.h
	${CMAKE_CURRENT_SOURCE_DIR}/ch264code.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/camera/camerads.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera/camerads.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/camera/convert.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera/convert.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/camera/icameracaptuer.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera/icameracaptuer.cpp
)

SOURCE_GROUP(${BUILD_PROJECT_NAME} FILES
	${CMAKE_CURRENT_SOURCE_DIR}/icamrea_main.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/ch264code.h
	${CMAKE_CURRENT_SOURCE_DIR}/ch264code.cpp
)

SOURCE_GROUP(camera FILES
	${CMAKE_CURRENT_SOURCE_DIR}/camera/camerads.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera/camerads.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/camera/convert.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera/convert.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/camera/icameracaptuer.h
	${CMAKE_CURRENT_SOURCE_DIR}/camera/icameracaptuer.cpp
)

#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})

#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")
if(WIN32)
	ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
	TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME}
		libvzlogging
		libsystemv
		ws2_32.lib
	)
endif()