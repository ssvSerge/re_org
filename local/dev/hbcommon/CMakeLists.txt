cmake_minimum_required(VERSION 3.4.1)

project(hbcommon VERSION 0.1 LANGUAGES CXX C)

find_package (Threads REQUIRED)

set(BASE_DIR ../)

set(CMAKE_BUILD_TYPE Debug)

get_filename_component(BASE_DIR_ABSOLUTE ${BASE_DIR} ABSOLUTE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_property(the_include_dirs DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "INCL_DIRS = ${the_include_dirs}")
set_property(DIRECTORY . PROPERTY INCLUDE_DIRECTORIES "")

set(LIB_TYPE STATIC)

file(GLOB SRC_FILES src/*.cpp)

add_library(hbcommon ${LIB_TYPE}
    ${SRC_FILES}
)

set_target_properties(hbcommon 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "."
)
# This gets rid of the nhlomann GCC 7.1 warning
target_compile_options(hbcommon PRIVATE "-Wno-psabi")

target_link_libraries (hbcommon
    LINK_PUBLIC
)
target_include_directories(hbcommon PUBLIC
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/ICypher/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
