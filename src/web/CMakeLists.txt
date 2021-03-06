cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "web_html")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "web_html")
endif()

#########################################################################
#INCLUDE_DIRECTORIES(${LIBVZNET_INCLUDE_DIR})
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
#LINK_DIRECTORIES(${LIBVZNET_LIBRARY_DIR})
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
#Step 3 : Add code source
IF(WIN32)
	SET(WIN32_LOAD_FILE
		${CMAKE_CURRENT_SOURCE_DIR}/index.html
		${CMAKE_CURRENT_SOURCE_DIR}/main.html
	
		${CMAKE_CURRENT_SOURCE_DIR}/page/preview.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/pc_year.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/pc_month.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/pc_day.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/log_download.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_vdo_enc.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_http_send.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_set_time.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_people_count.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_devinfo.html
		${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_dev_update.html
	)
ELSEIF()
	SET(WIN32_LOAD_FILE
	)
ENDIF()

SET(SRC_LIST 
	${WIN32_LOAD_FILE}

	${CMAKE_CURRENT_SOURCE_DIR}/web_main.cpp
)

SOURCE_GROUP(main FILES
	${CMAKE_CURRENT_SOURCE_DIR}/web_main.cpp
)

IF(WIN32)
SOURCE_GROUP(main_html FILES
	${CMAKE_CURRENT_SOURCE_DIR}/index.html
	${CMAKE_CURRENT_SOURCE_DIR}/main.html
)

SOURCE_GROUP(page_html FILES
	${CMAKE_CURRENT_SOURCE_DIR}/page/preview.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/pc_year.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/pc_month.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/pc_day.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/log_download.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_vdo_enc.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_http_send.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_set_time.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_people_count.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_devinfo.html
	${CMAKE_CURRENT_SOURCE_DIR}/page/cfg_dev_update.html
)
ENDIF()

#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")

#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
#set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
)