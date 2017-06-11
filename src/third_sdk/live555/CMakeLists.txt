# CMakeLists.txt in live

# includes
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/BasicUsageEnvironment/include
	${CMAKE_CURRENT_SOURCE_DIR}/groupsock/include
	${CMAKE_CURRENT_SOURCE_DIR}/liveMedia/include
	${CMAKE_CURRENT_SOURCE_DIR}/UsageEnvironment/include
)

ADD_SUBDIRECTORY(BasicUsageEnvironment)
ADD_SUBDIRECTORY(groupsock)
ADD_SUBDIRECTORY(liveMedia)
#ADD_SUBDIRECTORY(mediaServer)
#ADD_SUBDIRECTORY(proxyServer)
ADD_SUBDIRECTORY(UsageEnvironment)
#ADD_SUBDIRECTORY(testProgs)
