cmake_minimum_required(VERSION 3.4.1)

project(JEngineShell VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX AND NOT APPLE)
find_package (Threads REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-psabi -Wno-unknown-pragmas")
endif()

set(BASE_DIR ../)

get_filename_component(BASE_DIR_ABSOLUTE ${BASE_DIR} ABSOLUTE)

set(CMAKE_BUILD_TYPE Debug)

add_executable(JEngineShell
    ./src/main.cpp
    ./src/JEngineShell.cpp
    ./src/JEngineExec.cpp
    ./src/V100CommandBroker_Shell.cpp
    ../hbcommon/src/logging.cpp
    ../hbcommon/src/base64.cpp
    ../hbcommon/src/hbbmp.cpp
    ../hbcommon/src/RemoteExec.cpp
    ../api_common/src/DataMgr.cpp
    ../api_common/src/CryptoMgrHid.cpp
    ../common/src/ICmd.cpp
    ../common/src/V100Cmd.cpp
    ../common/src/V100EncCmd.cpp
    ../common/src/V100IDCmd.cpp
    ../common/src/V100InternalCmd.cpp
    ../common/src/V100_HID.cpp
    ../TransCommon/src/CmdExecutiveCommon_HID.cpp
    ../TransCP001/src/CmdExecutiveCP001_HID.cpp
    ../VCOMCrypto/src/CommandHandler.cpp
    ../VCOMCrypto/src/IEncCmd.cpp
    $ENV{HID_FACE_DEPENDENCIES}/src/tdbg.cpp
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/../../common/src/vcom_tools.cpp>
)

target_compile_definitions(JEngineShell PUBLIC _TDBG_LEVEL=2)

target_link_libraries (JEngineShell
    LINK_PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    FirmwareConfigParser
    hbcommon
    HBSEClient
    ICypher
    SocketCommunLib
    stdc++fs
    rt
    HidFace
)
target_compile_definitions(JEngineShell PRIVATE MEM_NAMESPACE=AlgoMem __OVERRIDE_NEW__=0 )

target_link_directories(JEngineShell PUBLIC
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/../build/vision_plus/lib/Linux/arm64_vision>
)

target_include_directories(JEngineShell PUBLIC
    $<INSTALL_INTERFACE:include>
    $ENV{HID_FACE_DEPENDENCIES}/include
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/TransCP001/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/api_common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/hbcommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/socketComm/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/ICypher/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/TransCommon/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/common/include>
    $<BUILD_INTERFACE:${BASE_DIR_ABSOLUTE}/VCOMCrypto/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE src)
