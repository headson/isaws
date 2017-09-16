cmake_minimum_required(VERSION 2.8)
#########################################################################

if(WIN32)
    SET(BUILD_PROJECT_NAME "libplatform")
elseif(UNIX)
    SET(BUILD_PROJECT_NAME "platform")
endif()

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include
)

LINK_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/mpp/lib 
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

SET(HI3518E200_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/acodec.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/mpi_region.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_ae_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_af_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_awb_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_3a.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_adec.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_aenc.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_ai.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_aio.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_ao.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_isp.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_ive.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_region.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_sns.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_sys.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_vb.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_vda.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_vdec.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_venc.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_vi.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_video.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_vo.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_comm_vpss.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/include/hi_common.h

	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_audio.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_isp.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_ive.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_ive.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_sys.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_venc.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_vi.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_vo.c
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/common/sample_comm_vpss.c
	
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/sendframe.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/vpsschnosd.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/vpsschnosd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/vpsschndump.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/caudiocatch.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/caudiocatch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/cvideocatch.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/cvideocatch.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/cgpioevent.h
	${CMAKE_CURRENT_SOURCE_DIR}/hi3518ev200/cgpioevent.cpp
)

SET(PROJECT_SOURCE_LIST	
	${CMAKE_CURRENT_SOURCE_DIR}/basedefines.h
	
	${CMAKE_CURRENT_SOURCE_DIR}/cpcalgctrl.h
	${CMAKE_CURRENT_SOURCE_DIR}/cpcalgctrl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ondebug.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/onaction.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/cextdevevt.h
	${CMAKE_CURRENT_SOURCE_DIR}/cextdevevt.cpp
	
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.h
	${CMAKE_CURRENT_SOURCE_DIR}/clistenmessage.cpp
)

SOURCE_GROUP(hi3518ev200 FILES
	${HI3518E200_SRC}
) 

SOURCE_GROUP(listen FILES
	${PROJECT_SOURCE_LIST}
) 

IF(WIN32)
    IF(MSVC12)
        ADD_LIBRARY(${BUILD_PROJECT_NAME} 
            ${PROJECT_SOURCE_LIST}
            ${HI3518E200_SRC} 
        )
    ELSEIF(MSVC10)
    ENDIF()
ELSEIF(UNIX)
    ADD_DEFINITIONS(-fpermissive)
    IF(CMAKE_BUILD_PLATFORM MATCHES LINUX)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES IMX6Q)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES HI3516)
    ELSEIF(CMAKE_BUILD_PLATFORM MATCHES HI3518)
        ADD_DEFINITIONS(
            -Dhi3518ev200
            -DHICHIP=0x3518E200
            -DSENSOR_TYPE=APTINA_AR0130_DC_720P_30FPS
            -DCHIP_ID=CHIP_HI3518E_V200
            -DHI_DEBUG 
            -DHI_XXXX 
            -DISP_V2
        )
        ADD_LIBRARY(${BUILD_PROJECT_NAME} 
            ${PROJECT_SOURCE_LIST}
            ${HI3518E200_SRC} 
        )
    ENDIF()
ENDIF()
#########################################################################
MESSAGE(STATUS "Step 4 : Add project define")


#########################################################################
MESSAGE(STATUS "Step 5 : Add executable or library target")

SET_PROPERTY(TARGET ${BUILD_PROJECT_NAME} 
	PROPERTY FOLDER ${PROJECT_SET_NAME_LIB})

