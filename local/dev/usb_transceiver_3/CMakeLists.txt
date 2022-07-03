cmake_minimum_required(VERSION 3.4.1)

project(usb_transceiver_3 VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX AND NOT APPLE)
find_package (Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-psabi -Wno-unknown-pragmas -pedantic -O0 -g3 -ggdb ")
endif()

set(BASE_DIR ../)

get_filename_component(BASE_DIR_ABSOLUTE ${BASE_DIR} ABSOLUTE)

set(CMAKE_BUILD_TYPE Debug)

add_executable(usb_transceiver_3
    ./src/epconst.c 
    ./src/usb_transceiver.cpp 
    ./src/usb_transport_cancel.cpp 
    ./src/usb_transport_command.cpp 
    ./src/usb_transport_device.cpp 
    ./src/usb_transport_io.cpp 
    ./src/usb_transport_phantom_read.cpp 
    ./src/usb_transport_rx_header.cpp 
    ./src/usb_transport_rx_payload.cpp 
    ./src/usb_transport_setup.cpp 
    ./src/usb_transport_spinup.cpp 
    ./src/usb_transport_threads.cpp 
    ./src/usb_transport_tx.cpp 
)

target_link_libraries (usb_transceiver_3
    LINK_PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    aio
    stdc++fs
    rt
    hbcommon
)

target_link_directories(usb_transceiver_3 PUBLIC
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/libaio/src>
)

target_include_directories(usb_transceiver_3 PUBLIC
    $ENV{HID_FACE_DEPENDENCIES}/include
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/usb_transceiver_3/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/libaio/src>
    PRIVATE src)
