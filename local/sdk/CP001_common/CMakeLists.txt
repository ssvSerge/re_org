cmake_minimum_required(VERSION 3.4.1)

set(PROJECT_NAME CP001Common)
project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message("${CMAKE_BUILD_TYPE}")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_library(${PROJECT_NAME} STATIC
    src/DeviceKeyManager.cpp
    src/HostCryptoMgr.cpp
    src/V100_HID.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set(PREPROCESSOR_FLAGS "")
if (WIN32)
set(PREPROCESSOR_FLAGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_AFXDLL
    -DUNICODE
    -D_UNICODE
    -D_CRT_SECURE_NO_WARNINGS
)
add_definitions(${PREPROCESSOR_FLAGS})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/include/hidfacelib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../common/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE   VCOMCrypto
    PRIVATE   ICypher
)
