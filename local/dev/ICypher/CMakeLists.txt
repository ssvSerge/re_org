cmake_minimum_required(VERSION 3.10.0)

set(PROJECT_NAME "ICypher")

project(${PROJECT_NAME} VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BASE_DIR ../)

set(CMAKE_BUILD_TYPE Debug)

get_filename_component(BASE_DIR_ABS "${BASE_DIR}" ABSOLUTE)

file(GLOB C_SRC src/*.c)
file(GLOB CXX_SRC src/*.cpp)

set(BUILD_LIB_TYPE STATIC)

add_library(${PROJECT_NAME} ${BUILD_LIB_TYPE}
   ${C_SRC}
   ${CXX_SRC}
   ${BASE_DIR_ABS}/common/src/IMemMgr.cpp
   ${BASE_DIR_ABS}/common/src/MemoryManager.cpp
)

message(STATUS "BASE_DIR_ABS = ${BASE_DIR_ABS}")

target_compile_definitions(${PROJECT_NAME} PRIVATE MEM_NAMESPACE=AlgoMem
__OVERRIDE_NEW__=0
POLARSSL_CONFIG_FILE="polarssl/lumi_config.h"
NO_STATIC_RANDOM
NO_FP)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/polarssl>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/usb_transceiver/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/../../common/include>
    $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/API>
    PRIVATE src)
