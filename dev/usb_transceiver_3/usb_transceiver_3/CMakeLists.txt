cmake_minimum_required(VERSION 3.4.1)

project(usb_transceiver VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX AND NOT APPLE)
find_package (Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-psabi -Wno-unknown-pragmas -fsanitize=address")
endif()

set(BASE_DIR ../)

get_filename_component(BASE_DIR_ABSOLUTE ${BASE_DIR} ABSOLUTE)

set(CMAKE_BUILD_TYPE Debug)

add_executable(usb_transceiver
    ./include/IMemMgr.h
    ./src/epconst.c
    ./src/epinit.cpp
    ./src/USBTransceiver.cpp
    ./src/VCOMTransceiver.cpp
    ./src/TransactionBroker.cpp
    ./src/UpdateManager.cpp
    ./src/V100CommandBroker_Device.cpp
    ${BASE_DIR}/VCOMCrypto/src/IEncCmd.cpp
    ${BASE_DIR}/common/src/V100Cmd.cpp
    ${BASE_DIR}/common/src/ICmd.cpp
    ${BASE_DIR}/common/src/V100IDCmd.cpp
    ${BASE_DIR}/common/src/V100InternalCmd.cpp
    ${BASE_DIR}/common/src/V100EncCmd.cpp
    ${BASE_DIR}/common/src/FileStructure.cpp
    ${BASE_DIR}/common/src/IMemMgr.cpp
    ${BASE_DIR}/common/src/MemoryManager.cpp
    ${BASE_DIR}/../../common/src/vcom_tools.cpp
)

target_link_libraries (usb_transceiver
    LINK_PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    aio
    hbcommon
    ICypher
    HBSEClient
    SocketCommunLib
    stdc++fs
    rt
    FirmwareConfigParser
    HidFace
)

target_compile_definitions(usb_transceiver PRIVATE MEM_NAMESPACE=AlgoMem
__OVERRIDE_NEW__=0
)

target_link_directories(usb_transceiver PUBLIC
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/libaio/src>
)

target_include_directories(usb_transceiver PUBLIC
    $ENV{HID_FACE_DEPENDENCIES}/include
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/../../lib/include/hidfacelib>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/ICypher/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE src)
