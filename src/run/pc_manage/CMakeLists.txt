cmake_minimum_required(VERSION 3.8.0)
#########################################################################
project(pc_manage)

SET(CMAKE_PREFIX_PATH $ENV{QT5_HOME})

# Basic environment setting
SET(BUILD_PROJECT_NAME "pc_manage")

# Find includes in corresponding build directories
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#########################################################################
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/sdk/ffmpeg/inc)
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
#Step 2 : Add library directories
MESSAGE(STATUS "Step 2 : Add library directories")
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/sdk/ffmpeg/lib)
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

#Qt5Core, Qt5DBus, Qt5Gui, QtWidgets, Qt5Multimedia, Qt5Network, Qt5OpenGL, Qt5Qml Qt5Quick, Qt5WebKit
# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5SQL)
find_package(Qt5Multimedia)

ADD_DEFINITIONS (${QT_DEFINITIONS})  

#########################################################################
#Step 3 : Add code source
qt5_add_resources(QRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.qrc
)

qt5_wrap_ui(UI_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevlistwidget.ui
	
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideoplayer.ui
	${CMAKE_CURRENT_SOURCE_DIR}/preview/clivewidget.ui
	
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdevcfgwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/civacfgwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdbgvdowidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cuplinkwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cvdocfgwidget.ui
	
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/clogindialog.ui
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/cmaindialog.ui
)

SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdatabase.h
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdatabase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/database/cusrinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdevinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdevinfo.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmydialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmydialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmytitlebar.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmytitlebar.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevicedetect.h
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevicedetect.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevlistwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevlistwidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideoplayer.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideoplayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preview/chttpflvclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/chttpflvclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideowidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preview/clivewidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/clivewidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdevcfgwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdevcfgwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/config/civacfgwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/civacfgwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdbgvdowidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdbgvdowidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/config/cuplinkwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cuplinkwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/config/cvdocfgwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cvdocfgwidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/clogindialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/clogindialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/cmaindialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/cmaindialog.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/appinc.h
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

SOURCE_GROUP(database FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdatabase.h
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdatabase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/database/cusrinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdevinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/database/cdevinfo.cpp
)

SOURCE_GROUP(common FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmydialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmydialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmytitlebar.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmytitlebar.cpp
)

SOURCE_GROUP(devtree FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevicedetect.h
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevicedetect.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevlistwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevlistwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/devtree/cdevlistwidget.cpp
)

SOURCE_GROUP(preview FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideoplayer.ui
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideoplayer.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideoplayer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preview/chttpflvclient.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/chttpflvclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideowidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/cvideowidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/preview/clivewidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/preview/clivewidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/preview/clivewidget.cpp
)

SOURCE_GROUP(config FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdevcfgwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdevcfgwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdevcfgwidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/config/cuplinkwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cuplinkwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cuplinkwidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/config/cvdocfgwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cvdocfgwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cvdocfgwidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/config/civacfgwidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/civacfgwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/civacfgwidget.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdbgvdowidget.ui
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdbgvdowidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/config/cdbgvdowidget.cpp
)

SOURCE_GROUP(maindialog FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/clogindialog.ui
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/clogindialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/clogindialog.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/cmaindialog.ui
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/cmaindialog.h
	${CMAKE_CURRENT_SOURCE_DIR}/maindialog/cmaindialog.cpp
)

SOURCE_GROUP(main FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/appinc.h
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.qrc
)

# Use the Widgets module from Qt 5.
ADD_EXECUTABLE(${BUILD_PROJECT_NAME} 
	${SRC_LIST}
	${UI_FILES}
	${QRC_FILES}
)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_RUN})

TARGET_LINK_LIBRARIES (${BUILD_PROJECT_NAME} 
	Qt5::Widgets 
	Qt5::Core 
	Qt5::Gui
	Qt5::Sql
	Qt5::Multimedia
	
	libcurlclient
	libcurl
	libvzconn
	libjsoncpp
	${VZPROJECT_LINK_LIB}
)
