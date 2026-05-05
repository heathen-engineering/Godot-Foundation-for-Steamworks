cmake_minimum_required(VERSION 3.16)
project(FoundationSteamworks)

set(STEAM_MAJOR 1 CACHE STRING "Steam SDK major version")
set(STEAM_MINOR 63 CACHE STRING "Steam SDK minor version")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(GODOT_CPP_PATH "/home/loden/Dev/GitHub/Godot-cpp" CACHE PATH "Path to Godot-cpp bindings")
set(STEAM_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/sdk/public" CACHE PATH "Path to Steamworks SDK public folder")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" TYPE_STRING)

if(WIN32)
    set(PLATFORM "windows")
elseif(APPLE)
    set(PLATFORM "macos")
else()
    set(PLATFORM "linux")
endif()

set(ARCH "x86_64")

include_directories(
    ${GODOT_CPP_PATH}/gen/include
    ${GODOT_CPP_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/public
    ${STEAM_SDK_PATH}
)

message(STATUS "Building: ${CMAKE_BUILD_TYPE} on ${PLATFORM} ${ARCH}")
message(STATUS "GODOT_CPP_PATH: ${GODOT_CPP_PATH}")
message(STATUS "STEAM_SDK_PATH: ${STEAM_SDK_PATH}")

file(GLOB_RECURSE SOURCES "src/*.*")

add_library(FoundationSteamworks SHARED ${SOURCES})

if(WIN32)
    target_link_libraries(FoundationSteamworks PRIVATE ${GODOT_CPP_PATH}/bin/libgodot-cpp.win.template_release.x86_64${CMAKE_STATIC_LIBRARY_SUFFIX})
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/win64/steam_api.lib")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(FoundationSteamworks PRIVATE ${GODOT_CPP_PATH}/bin/libgodot-cpp.linux.template_release.x86_64${CMAKE_STATIC_LIBRARY_SUFFIX})
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/linux64/libsteam_api.so")
elseif(APPLE)
    target_link_libraries(FoundationSteamworks PRIVATE ${GODOT_CPP_PATH}/bin/libgodot-cpp.mac.template_release.x86_64${CMAKE_STATIC_LIBRARY_SUFFIX})
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/osx/libsteam_api.dylib")
endif()

set_target_properties(FoundationSteamworks
    PROPERTIES
    PREFIX ""
    LINK_FLAGS "-rdynamic"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    OUTPUT_NAME "libFoundationSteamworks.${PLATFORM}.${TYPE_STRING}.${ARCH}"
)

target_compile_definitions(FoundationSteamworks
    PRIVATE
    STEAM_MAJOR=${STEAM_MAJOR}
    STEAM_MINOR=${STEAM_MINOR}
)
