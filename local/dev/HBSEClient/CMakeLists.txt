cmake_minimum_required(VERSION 3.10.0)

set(PROJECT_NAME "HBSEClient")

project(${PROJECT_NAME} VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_BUILD_TYPE Debug)

set(BASE_DIR "../")

get_filename_component(BASE_DIR_ABS "${BASE_DIR}" ABSOLUTE)

if (${SENGINE_BATCH_BUILD})
set(BUILD_LIB_TYPE OBJECT)
else()
set(BUILD_LIB_TYPE STATIC)
endif()

add_library(${PROJECT_NAME} ${BUILD_LIB_TYPE}
    src/HBSEClient.cpp
)

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    $ENV{HID_FACE_DEPENDENCIES}/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABS}/../../common/include>
)
