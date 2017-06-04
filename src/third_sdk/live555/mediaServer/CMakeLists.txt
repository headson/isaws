# CMakeLists.txt mediaServer directory

# headers
SET(MEDIA_SERVER_HDRS
DynamicRTSPServer.hh
)

# source files for mediaServer
SET(MEDIA_SERVER_SRCS 
DynamicRTSPServer.cpp
live555MediaServer.cpp
)

ADD_EXECUTABLE(mediaServer ${MEDIA_SERVER_HDRS} ${MEDIA_SERVER_SRCS})
set_property(TARGET mediaServer PROPERTY FOLDER ${PROJECT_SET_NAME_THIRD_PART})
TARGET_LINK_LIBRARIES(
mediaServer
BasicUsageEnvironment
groupsock
liveMedia
UsageEnvironment
ws2_32
winmm
) 
