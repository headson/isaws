
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set_property(GLOBAL PROPERTY USE_FOLDERS On)

SET(PROJECT_SET_NAME_TEST 					"test")
SET(PROJECT_SET_NAME_LIB 					"lib")
SET(PROJECT_SET_NAME_RUN 					"app")
SET(PROJECT_SET_NAME_CMAKE 					"cmake")
SET(PROJECT_SET_NAME_TEST_DEVICE_GROUP 	"test/devicegroup")
SET(FTP_DIR 								    "/mnt/hgfs/vscp/code/branches/ftp_dir")
SET (CMAKE_USE_RELATIVE_PATHS 				ON)

if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Setting the default build type with Debug" FORCE )
endif()