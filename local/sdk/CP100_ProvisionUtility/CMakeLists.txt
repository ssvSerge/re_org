cmake_minimum_required(VERSION 3.4.1)

set(PROJECT_NAME CP001ProvisionUtility)

project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX C)
set(SLN_NAME CP001)

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

add_executable(${PROJECT_NAME} CP100_ProvisionUtility.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES 
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_COMMIT}
    CXX_STANDARD 14
)

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

if (WIN32)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    CP001Common
    ICypher
    VCOMCrypto
    SetupAPI
)
else()
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    CP001Common
    ICypher
    VCOMCrypto
    dl
)
endif()
