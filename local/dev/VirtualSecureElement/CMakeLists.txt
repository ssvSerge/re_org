cmake_minimum_required(VERSION 3.10.0)

set(PROJECT_NAME "VirtualSELib")

project(${PROJECT_NAME} VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_BUILD_TYPE Debug)

set(BASE_DIR "../")
get_filename_component(BASE_DIR_ABS "${BASE_DIR}" ABSOLUTE)

file(GLOB SRC_FILES src/*.cpp)
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/../../common/src/vcom_tools.cpp>
)

target_link_libraries (${PROJECT_NAME}
    LINK_PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    pthread
    SocketCommunLib
    ICypher
    hbcommon
    stdc++fs
    rt
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/common/include>    
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/ICypher/include>
    PRIVATE src
)
