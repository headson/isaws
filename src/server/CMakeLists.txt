cmake_minimum_required(VERSION 2.8)

set(APP_SERVER_NAME server)

include_directories(${PROJECT_SOURCE_DIR}/common)

set(SERVER_SRC_LIST
        main.cpp)

add_executable(${APP_SERVER_NAME}
        ${SERVER_SRC_LIST})
target_link_libraries(${APP_SERVER_NAME}
        common)