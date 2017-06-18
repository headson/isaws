# CMakeLists.txt in groupsock

SET(GROUPSOCK_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/GroupEId.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Groupsock.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/GroupsockHelper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/inet.c
	${CMAKE_CURRENT_SOURCE_DIR}/IOHandlers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/NetAddress.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/NetInterface.cpp
)

SET(GROUPSOCK_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/GroupEId.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/Groupsock.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/GroupsockHelper.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/groupsock_version.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/IOHandlers.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/NetAddress.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/NetInterface.hh
	${CMAKE_CURRENT_SOURCE_DIR}/include/TunnelEncaps.hh
)

ADD_LIBRARY(groupsock STATIC ${GROUPSOCK_HEADERS} ${GROUPSOCK_SRCS})
set_property(TARGET groupsock PROPERTY FOLDER ${PROJECT_SET_NAME_THIRD_PART})
