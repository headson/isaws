cmake_minimum_required(VERSION 2.8)
#########################################################################

if(WIN32)
    SET(BUILD_PROJECT_NAME "libplatform")
elseif(UNIX)
    SET(BUILD_PROJECT_NAME "platform")
endif()

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include
)

LINK_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/lib
)

#########################################################################
MESSAGE(STATUS "Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
  message(STATUS "    include directory='${inc_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
  message(STATUS "    link directory='${link_dir}'")
endforeach()

#########################################################################
MESSAGE(STATUS "Step 3 : Add project code")
SET(PROJECT_SOURCE_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/acodec.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/dictionary.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_ae_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_af_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_awb_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_3a.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_adec.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_aenc.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_ai.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_aio.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_ao.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_isp.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_ive.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_region.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_sns.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_sys.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vb.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vda.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vdec.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_venc.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vgs.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vi.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_video.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vo.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vpss.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_common.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/common/loadbmp.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/loadbmp.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_audio.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_isp.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_ive.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_ive.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_sys.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vda.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_venc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vi.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vo.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vpss.c
	
	${CMAKE_CURRENT_SOURCE_DIR}/caudio.h
	${CMAKE_CURRENT_SOURCE_DIR}/caudio.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cvideo.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideo.cpp
)

SOURCE_GROUP(mpp_include FILES
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/acodec.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/dictionary.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_ae_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_af_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_awb_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_3a.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_adec.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_aenc.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_ai.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_aio.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_ao.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_isp.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_ive.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_region.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_sns.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_sys.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vb.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vda.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vdec.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_venc.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vgs.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vi.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_video.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vo.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_comm_vpss.h
	${CMAKE_CURRENT_SOURCE_DIR}/mpp/include/hi_common.h
)

SOURCE_GROUP(common FILES
	${CMAKE_CURRENT_SOURCE_DIR}/common/loadbmp.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/loadbmp.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_audio.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_isp.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_ive.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_ive.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_sys.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vda.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_venc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vi.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vo.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sample_comm_vpss.c
)

SOURCE_GROUP(audio_video FILES
	${CMAKE_CURRENT_SOURCE_DIR}/caudio.h
	${CMAKE_CURRENT_SOURCE_DIR}/caudio.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/cvideo.h
	${CMAKE_CURRENT_SOURCE_DIR}/cvideo.cpp
)
#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")
ADD_DEFINITIONS(
	-DHICHIP=0x3516A100
	-DSENSOR_TYPE=SONY_IMX178_LVDS_5M_30FPS
)

#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")
ADD_LIBRARY(${BUILD_PROJECT_NAME} ${PROJECT_SOURCE_LIST})

SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})
