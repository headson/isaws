# CMakeLists.txt in BasicUsageEnvironment

SET(BASIC_USAGE_ENVIRONMENT_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/BasicHashTable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BasicTaskScheduler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BasicTaskScheduler0.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BasicUsageEnvironment.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BasicUsageEnvironment0.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DelayQueue.cpp
)

SET(BASIC_USAGE_ENVIRONMENT_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/BasicHashTable.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/BasicUsageEnvironment.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/BasicUsageEnvironment0.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/BasicUsageEnvironment_version.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/DelayQueue.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/HandlerSet.hh
)

ADD_LIBRARY( BasicUsageEnvironment STATIC ${BASIC_USAGE_ENVIRONMENT_HEADERS} ${BASIC_USAGE_ENVIRONMENT_SRCS})
SET_PROPERTY(TARGET BasicUsageEnvironment PROPERTY FOLDER ${PROJECT_SET_NAME_THIRD_PART})
