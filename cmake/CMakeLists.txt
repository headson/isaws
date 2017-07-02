cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting
SET(BUILD_PROJECT_NAME "cmake_settings")

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
	${CMAKE_CURRENT_SOURCE_DIR}/global_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/win_vs2010_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/win_vs2013_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/arm_imx6q_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/arm_hi3516_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/arm_hi3518_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake_main.cpp
)

SOURCE_GROUP(cmake FILES
	${CMAKE_CURRENT_SOURCE_DIR}/global_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/win_vs2010_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/win_vs2013_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/arm_imx6q_environment_settings.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/arm_hi3516_environment_settings.cmake 
	${CMAKE_CURRENT_SOURCE_DIR}/arm_hi3518_environment_settings.cmake
)

#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_CMAKE})