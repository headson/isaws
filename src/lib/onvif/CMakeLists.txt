cmake_minimum_required(VERSION 2.8)
#########################################################################

# Basic environment setting

if(WIN32)
	SET(BUILD_PROJECT_NAME "libonvif")
elseif(UNIX)
	SET(BUILD_PROJECT_NAME "onvif")
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
SET(SRC_LIST 
	${CMAKE_CURRENT_SOURCE_DIR}/bm/base64.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/hqueue.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/hqueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/ihash.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/linked_list.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/linked_list.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/ppstack.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/ppstack.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rc4.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rfc_md5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rfc_md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rqueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sha1.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_buf.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_buf.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_inc.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_log.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_log.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_os.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/util.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/util.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/word_analyse.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/word_analyse.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_auth.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_auth.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_cln.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_cln.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_parse.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_rx.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/hxml.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/hxml.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_analytics.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_analytics.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_api.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_api.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cfg.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cfg.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cm.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cm.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_device.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_device.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_err.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_event.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_event.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_image.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_image.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_local.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_local.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_local_datastruct.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_media.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_media.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_pkt.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_pkt.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_probe.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_probe.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_ptz.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_ptz.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_recording.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_recording.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_timer.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_timer.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_util.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_util.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/soap.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/soap_parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/soap_parser.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/xml_node.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/xml_node.h
	${CMAKE_CURRENT_SOURCE_DIR}/localsdk/VzLocalSdk.c
	${CMAKE_CURRENT_SOURCE_DIR}/localsdk/VzLocalSdk.h
	${CMAKE_CURRENT_SOURCE_DIR}/localsdk/VzLocalSdk_DataDefine.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/json_onvifinfo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/json_onvifinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_cfgfile_mng.c
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_cfgfile_mng.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_dispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_dispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_kvdb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_kvdb.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_pro_msg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_pro_msg.h
	)

SOURCE_GROUP(bm FILES
	${CMAKE_CURRENT_SOURCE_DIR}/bm/base64.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/hqueue.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/hqueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/ihash.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/linked_list.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/linked_list.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/ppstack.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/ppstack.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rc4.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rfc_md5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rfc_md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/rqueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sha1.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sha1.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_buf.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_buf.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_inc.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_log.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_log.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/sys_os.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/util.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/util.h
	${CMAKE_CURRENT_SOURCE_DIR}/bm/word_analyse.c
	${CMAKE_CURRENT_SOURCE_DIR}/bm/word_analyse.h
	)
	
SOURCE_GROUP(onvif FILES
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_auth.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_auth.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_cln.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_cln.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_parse.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/http_rx.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/hxml.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/hxml.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_analytics.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_analytics.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_api.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_api.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cfg.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cfg.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cm.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_cm.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_device.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_device.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_err.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_event.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_event.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_image.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_image.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_local.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_local.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_local_datastruct.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_media.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_media.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_pkt.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_pkt.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_probe.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_probe.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_ptz.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_ptz.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_recording.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_recording.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_timer.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_timer.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_util.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif_util.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/soap.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/soap_parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/soap_parser.h
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/xml_node.c
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/xml_node.h
	)	
	
SOURCE_GROUP(localsdk FILES
	${CMAKE_CURRENT_SOURCE_DIR}/localsdk/VzLocalSdk.c
	${CMAKE_CURRENT_SOURCE_DIR}/localsdk/VzLocalSdk.h
	${CMAKE_CURRENT_SOURCE_DIR}/localsdk/VzLocalSdk_DataDefine.h	
	)
	
SOURCE_GROUP(cfgpro FILES
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/json_onvifinfo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/json_onvifinfo.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_cfgfile_mng.c
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_cfgfile_mng.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_dispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_dispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_kvdb.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_kvdb.h
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_pro_msg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cfgpro/onvif_pro_msg.h
	)


#########################################################################
#Step 4 : Add PROJECT define 
MESSAGE(STATUS "Step 4 : Add code source")
ADD_DEFINITIONS(-DPROFILE_G_SUPPORT
	-DVIDEO_ANALYTICS
	-DONVIF
    -DONVIF_SEPARATE_CFG
    -DONVIF_USB_LAN
	-DNEW_ONVIF
	)
#########################################################################
#Step 5 : Add executable or library target
MESSAGE(STATUS "Step 5 : Add executable or library target")
#ADD_executable(${BUILD_PROJECT_NAME} ${SRC_LIST})
ADD_LIBRARY(${BUILD_PROJECT_NAME} STATIC ${SRC_LIST})
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
#########################################################################
#Step 6 : link with other library
MESSAGE(STATUS "Step 6 : link with other library")

TARGET_LINK_LIBRARIES(${BUILD_PROJECT_NAME} 
)