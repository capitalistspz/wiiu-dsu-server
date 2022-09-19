cmake_minimum_required(VERSION 3.22)
project(dsu_controller CXX)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE HEADER_FILES src/*.hpp src/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_compile_definitions(APPLICATION_NAME="DSU_CONTROLLER")

message(STATUS "${HEADER_FILES} ${SOURCE_FILES}")
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})


wut_create_rpx(${PROJECT_NAME})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/helloworld.rpx"
        DESTINATION "${CMAKE_INSTALL_PREFIX}")