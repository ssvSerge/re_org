cmake_minimum_required(VERSION 3.10.0)
set(BUILD_ROOT ../) 
set(PROJECT_NAME SocketCommunLib)
project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

get_filename_component(BUILD_ROOT_ABSOLUTE ${BUILD_ROOT} ABSOLUTE)
message(STATUS "BUILD_ROOT = ${BUILD_ROOT_ABSOLUTE}")

message("Building SEngine Shell...")
unset(LINUX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	message( "Detecting platform target type.")

	if(ANDROID)
		if( NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
			message( FATAL_ERROR "Unrecognized Android Build System.")
		endif()
		message("--- PLATFORM TARGET = ANDROID")
	else()
		set(LINUX 1)
		message("--- PLATFORM TARGET = LINUX")
	endif()
endif()

if ( LINUX )
	find_library(LIBUSB_LIBRARY NAMES usb-1.0 PATH_SUFFIXES "lib" "lib32" "lib64" PATHS ${BUILD_ROOT_ABSOLUTE}/libusb/lib/armv8-a)
	if( NOT LIBUSB_LIBRARY)
		message(FATAL_ERROR "LIBUSB_LIBRARY library not found.  Exiting.")
	else()
		message("FOUND LIBUSB_LIBRARY = ${LIBUSB_LIBRARY}")
	endif()
endif()

if (SENGINE_BATCH_BUILD)
set(LIB_TYPE OBJECT)
else()
set(LIB_TYPE STATIC)
endif()


add_library(${PROJECT_NAME} ${LIB_TYPE}
    src/SocketServer.cpp
    src/SocketClient.cpp
)

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    $ENV{HID_FACE_DEPENDENCIES}/include
    $<BUILD_INTERFACE:${BUILD_ROOT_ABSOLUTE}/hbcommon/include>
    $<BUILD_INTERFACE:${BUILD_ROOT_ABSOLUTE}/common/include>
    $<BUILD_INTERFACE:${BUILD_ROOT_ABSOLUTE}/socketComm/include>
    $<BUILD_INTERFACE:${BUILD_ROOT_ABSOLUTE}/../../common/include>
    $<BUILD_INTERFACE:${LIBUSB_INCLUDE_DIR}>
)