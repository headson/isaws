cmake_minimum_required(VERSION 2.8)
#########################################################################

if(WIN32)
	SET(BUILD_PROJECT_NAME "libinc")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "inc")
endif()

#########################################################################
MESSAGE(STATUS "Print the include directores")
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
MESSAGE(STATUS "Step 3 : Add code source")
SET(SRC_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/verror.h
	${CMAKE_CURRENT_SOURCE_DIR}/vtypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/vtypes.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/vboost.h
	${CMAKE_CURRENT_SOURCE_DIR}/vdefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/vmessage.h
)

SOURCE_GROUP(base FILES
	${CMAKE_CURRENT_SOURCE_DIR}/verror.h
	${CMAKE_CURRENT_SOURCE_DIR}/vtypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/vtypes.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/vboost.h
	${CMAKE_CURRENT_SOURCE_DIR}/vdefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/vmessage.h
)

#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB}
)
