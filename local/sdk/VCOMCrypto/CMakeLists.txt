cmake_minimum_required(VERSION 3.4.1)

set(PROJECT_NAME VCOMCrypto)

project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX C)
set(SLN_NAME CP001)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT DEFINED VERSION_MAJOR OR NOT DEFINED VERSION_MINOR OR NOT DEFINED VERSION_BUILD OR NOT DEFINED VERSION_COMMIT )
    set(VERSION_MAJOR 2)
    set(VERSION_MINOR 70)
    set(VERSION_BUILD 0)
    set(VERSION_COMMIT 33454)
    message("Version Information not found, defaulting to: v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}.${VERSION_COMMIT}")
endif()

include_directories(
    ./include
)
if(NOT WIN32)
include_directories(
    /usr/include/libusb-1.0/
)
endif()
file( GLOB SRC_FILES
    ./src/*.cpp
    ./src/*.c
)
if (WIN32)
    foreach(SRCITEM ${SRC_FILES})
        if (${SRCITEM} MATCHES "TransportLibUSB\.cpp")
            list(REMOVE_ITEM SRC_FILES ${SRCITEM})
        endif()
    endforeach()
else()
    foreach(SRCITEM ${SRC_FILES})
        if (${SRCITEM} MATCHES "TransportUSB\.cpp" OR
            ${SRCITEM} MATCHES "LDSNamedPipe\.cpp" OR
            ${SRCITEM} MATCHES "usbdriver\.cpp")
            list(REMOVE_ITEM SRC_FILES ${SRCITEM})
        endif()
    endforeach()
endif()

add_library(${PROJECT_NAME} STATIC 
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES 
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD} )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

set(PREPROCESSOR_FLAGS "")
if (WIN32)
set(PREPROCESSOR_FLAGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_AFXDLL
    -D__SUPPORT_WIN32_USB__=1
    -D_VCOM_EXAMPLE=1
    -D_USRDLL
    -DUNICODE
    -D_UNICODE
    -DVCOMCORE_EXPORTS
    -D_CRT_SECURE_NO_DEPRECATE
    -D_VCOM_CRYPTO
)
else()
set(PREPROCESSOR_FLAGS
    -D_VCOM_CRYPTO
    -D_GCC=1
    -D_LIBUSB_1_0_
    -D__SUPPORT_WIN32_USB__
)
endif()
add_definitions(${PREPROCESSOR_FLAGS})

target_link_libraries(
    ${PROJECT_NAME}
    CP001Common
)

if(NOT WIN32)
   target_link_libraries(${PROJECT_NAME} usb-1.0)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/../../lib/include/hidfacelib>
        $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/../../common/include>
        $<INSTALL_INTERFACE:include>
)