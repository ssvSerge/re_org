cmake_minimum_required(VERSION 3.4.1)

set(PROJECT_NAME ICypher)

project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX C )
set(SLN_NAME HYB02)

if (NOT DEFINED VERSION_MAJOR OR NOT DEFINED VERSION_MINOR OR NOT DEFINED VERSION_COMMIT )
    set(VERSION_MAJOR 2)
    set(VERSION_MINOR 70)
    set(VERSION_COMMIT 33454)
    message("Version Information not found, defaulting to: v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_COMMIT}")
endif()

message("${CMAKE_BUILD_TYPE}")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#-DNO_STATIC_RANDOM -DPOLARSSL_CONFIG_FILE=\"polarssl/lumi_config.h\"

add_definitions(
    -DNO_STATIC_RANDOM
    -DPOLARSSL_CONFIG_FILE="polarssl/lumi_config.h"
)

file( GLOB SRC_FILES
    ./src/*.cpp
    ./src/*.c
)
add_library(${PROJECT_NAME} STATIC 
    ${SRC_FILES}
)

add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -D_AFXDLL
)
#add_definitions(${PREPROCESSOR_FLAGS})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD} )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

target_link_libraries(${PROJECT_NAME} VCOMCrypto)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src
)