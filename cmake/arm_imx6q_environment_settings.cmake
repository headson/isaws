include(CMakeForceCompiler)
# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
set( CMAKE_SYSTEM_PROCESSOR arm ) 
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /home/davinci/dm3730/dvsdk4_03/linux-devkit/arm-arago-linux-gnueabi/bin/gcc)
SET(CMAKE_CXX_COMPILER /home/davinci/dm3730/dvsdk4_03/linux-devkit/arm-arago-linux-gnueabi/bin/g++)
SET(CMAKE_STRIP /home/davinci/dm3730/dvsdk4_03/linux-devkit/arm-arago-linux-gnueabi/bin/strip)

#CMAKE_FORCE_C_COMPILER(arm-arago-linux-gnueabi-gcc GNU)
#CMAKE_FORCE_CXX_COMPILER(arm-arago-linux-gnueabi-g++ GNU)
# where is the target environment 
#SET(CMAKE_FIND_ROOT_PATH  "/mnt/hgfs/Share/cmake project/libvznet")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(VZ_S_BASE_DIR "/mnt/hgfs/work/vz_s_baseclass")
SET(BOOST_1_53_DIR "/mnt/hgfs/work/boost_1_53_0")
SET(VSCP "/mnt/hgfs/work/vscp/code/branches/sip")
SET(OSA "/mnt/hgfs/work/event_server/osa/_linux")
SET(INTERFACE_INCLUDE_DIR "/mnt/hgfs/work/build/pub/interface_newhwinfo/include")
SET(INTERFACE_LIBRARY_DIR "/mnt/hgfs/work/build/pub/interface_newhwinfo/lib/arm3730/lib")
SET(CMEM_DIR "/home/davinci/dm3730/dvsdk4_03/linuxutils_2_26_02_05/packages/ti/sdo/linuxutils/cmem")
SET(LIB_CURL ${PROJECT_SOURCE_DIR}/src/third_part/curl)

SET(VZPROJECT_INCLUDE_DIR
	${LIB_CURL}/include
	${PROJECT_BINARY_DIR}/src/third_part/curl/include/curl
	${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/src
	${VZ_S_BASE_DIR}/zeromq/libzmq/include
	${BOOST_1_53_DIR}
	# Add Json DIR
	${VZ_S_BASE_DIR}/app_headers
	${VZ_S_BASE_DIR}/jsoncpp/include
	${PROJECT_SOURCE_DIR}/src/lib
	#${VZ_S_BASE_DIR}/Vz_head
    ${INTERFACE_INCLUDE_DIR}
    ${GLOG_DIR}/builds/arm6446
    ${GLOG_DIR}/src
)

MESSAGE(STATUS "       Add LIBRARY PATH AND NAME")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	MESSAGE(STATUS "       Debug Mode")
	SET(VZPROJECT_LIBRARY_DIR
		${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/arm3730
		${BOOST_1_53_DIR}/stage/lib
		# Add Json library
		${VZ_S_BASE_DIR}/jsoncpp/lib/arm3730
		${VZ_S_BASE_DIR}/zeromq/libzmq/lib/arm3730
		${PROJECT_SOURCE_DIR}/lib/arm3730
	    ${PROJECT_SOURCE_DIR}/src/osa/_linux/lib/arm3730
		/home/davinci/dm3730/dvsdk4_03/linux-devkit/arm-arago-linux-gnueabi/usr/include
	    ${VSCP}/lib/arm3730
		)
	SET(VZPROJECT_LINK_LIB
		libglog.a
		# Add Json library
		libboost_thread.a
		libboost_system.a
		libboost_random.a
        libeventserver.a
        libosa.a
        libblacklistdb.a
		libsqlite3.a
        libnetsqlite.a
        libzmq.a
        libvznetdp.so
		libjsoncpp.a
        ${INTERFACE_DIR}/lib/vz_hwi_sharemem.a
        ${INTERFACE_DIR}/lib/vz_sharemem.a
        ${INTERFACE_DIR}/lib/getSN.a
        ${INTERFACE_DIR}/lib/dm3730_fs8816.a
        ${INTERFACE_DIR}/lib/dm3730_gpio.a
        ${INTERFACE_DIR}/lib/ApproDrvMsg.a
        ${INTERFACE_DIR}/lib/dm3730_FS8816V2.88.a
        ${INTERFACE_DIR}/lib/sem_util.a
        ${INTERFACE_DIR}/lib/msg_util.a
        ${INTERFACE_DIR}/lib/file_msg_drv.a
        ${INTERFACE_DIR}/lib/share_mem.a
        ${INTERFACE_DIR}/lib/onvif_state.a
        ${CMEM_DIR}/lib/cmem.a470MV
        -lcrypto
        -Wl,-rpath=.
		pthread
		rt
		)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	MESSAGE(STATUS "       RELEASE MODE")
	MESSAGE(STATUS "       UNIX")
	SET(VZPROJECT_LIBRARY_DIR
		${VZ_S_BASE_DIR}/vz_log/glog-0.3.3/lib/arm3730
		${BOOST_1_53_DIR}/stage/lib
		# Add Json library
		${VZ_S_BASE_DIR}/jsoncpp/lib/arm3730
		${VZ_S_BASE_DIR}/zeromq/libzmq/lib/arm3730
		${PROJECT_SOURCE_DIR}/lib/arm3730
		/home/davinci/dm3730/dvsdk4_03/linux-devkit/arm-arago-linux-gnueabi/usr/include
	    ${VSCP}/lib/arm3730
		)
	SET(VZPROJECT_LINK_LIB
		libglog.a
		# Add Json library
		libboost_thread.a
		libboost_system.a
        libboost_chrono.a
		libboost_random.a
		libsqlite3.a
		libjsoncpp.a
        libzmq.a
        ${INTERFACE_LIBRARY_DIR}/vz_hwi_sharemem.a
        ${INTERFACE_LIBRARY_DIR}/vz_sharemem.a
        ${INTERFACE_LIBRARY_DIR}/getSN.a
        ${INTERFACE_LIBRARY_DIR}/dm3730_fs8816.a
        ${INTERFACE_LIBRARY_DIR}/dm3730_gpio.a
        ${INTERFACE_LIBRARY_DIR}/ApproDrvMsg.a
        ${INTERFACE_LIBRARY_DIR}/dm3730_FS8816V2.88.a
        ${INTERFACE_LIBRARY_DIR}/sem_util.a
        ${INTERFACE_LIBRARY_DIR}/msg_util.a
        ${INTERFACE_LIBRARY_DIR}/file_msg_drv.a
        ${INTERFACE_LIBRARY_DIR}/share_mem.a
        ${INTERFACE_LIBRARY_DIR}/onvif_state.a
        ${CMEM_DIR}/lib/cmem.a470MV
        -lcrypto
        -Wl,-rpath=.
		pthread
		rt
		)
ENDIF()

MESSAGE(STATUS "SETP 4 : ADD CODE SOURCE")

ADD_DEFINITIONS(-DGOOGLE_GLOG_DLL_DECL=
	-DO2
    -D_LINUX
    -DDM3730
    -DONVIF
    -DONVIF_SEPARATE_CFG
    -DONVIF_USB_LAN
    -DLINK_8816
    -DWITH_DOM
    -DGENERIC_RELAY
    -DSVN_REVISION=$(SVN_REVISION)
	)
    
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/bin/arm3730")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/arm3730")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${PROJECT_SOURCE_DIR}/lib/arm3730")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/bin/arm3730")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/arm3730")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG  "${PROJECT_SOURCE_DIR}/lib/arm3730")

# With Release properties
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/bin/arm3730")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/arm3730")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/lib/arm3730")


