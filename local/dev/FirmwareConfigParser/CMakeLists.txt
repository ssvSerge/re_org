
set(BUILD_ROOT ../)
cmake_minimum_required(VERSION 3.4.1)

# Name of Project
set(PROJECT_NAME FirmwareConfigParser)
project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_BUILD_TYPE Debug)

get_filename_component(BUILD_ROOT ${BUILD_ROOT} ABSOLUTE)
message("Build root = ${BUILD_ROOT}")

file(GLOB LIB_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)


set(LIB_TYPE STATIC)

add_library(${PROJECT_NAME} ${LIB_TYPE}
    ${LIB_SRC}
)

# This gets rid of the nhlomann GCC 7.1 warning
target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-psabi")

target_include_directories(${PROJECT_NAME} PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>   
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/ICypher/include>
)

