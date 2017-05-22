cmake_minimum_required(VERSION 2.8)

set(APP_PLATFORM_NAME platform)

include_directories(${PROJECT_SOURCE_DIR}/common)

set(PLATFORM_SRC_LIST
        main.cpp)

add_executable(${APP_PLATFORM_NAME}
        ${PLATFORM_SRC_LIST})
target_link_libraries(${APP_PLATFORM_NAME}
        common)