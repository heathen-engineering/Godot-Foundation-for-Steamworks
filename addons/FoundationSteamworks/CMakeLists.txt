cmake_minimum_required(VERSION 3.16)
project(FoundationSteamworks)

set(STEAM_MAJOR 1 CACHE STRING "Steam SDK major version")
set(STEAM_MINOR 63 CACHE STRING "Steam SDK minor version")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Defaults to the godot-cpp git submodule vendored at the repo root (../../godot-cpp
# relative to this file, i.e. <repo>/godot-cpp). Override with -DGODOT_CPP_PATH=...
# to build against a different checkout.
set(GODOT_CPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../godot-cpp" CACHE PATH "Path to Godot-cpp bindings")

# The Steamworks SDK is never committed to this repo (Valve's SDK license prohibits
# redistributing the SDK source/headers/import libs). Vendor it locally at
# include/sdk/ (see include/SteamSDKGoesHere.md) or let CI fetch it privately.
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

message(STATUS "Building: ${CMAKE_BUILD_TYPE} on ${PLATFORM} ${ARCH}")
message(STATUS "GODOT_CPP_PATH: ${GODOT_CPP_PATH}")
message(STATUS "STEAM_SDK_PATH: ${STEAM_SDK_PATH}")

if(NOT EXISTS "${GODOT_CPP_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "godot-cpp not found at ${GODOT_CPP_PATH}. Did you run 'git submodule update --init --recursive'?")
endif()
if(NOT EXISTS "${STEAM_SDK_PATH}/steam/steam_api.h")
    message(FATAL_ERROR "Steamworks SDK not found at ${STEAM_SDK_PATH}. See include/SteamSDKGoesHere.md.")
endif()

# Build godot-cpp as part of this build (no separate SCons pre-build step needed).
add_subdirectory(${GODOT_CPP_PATH} ${CMAKE_BINARY_DIR}/godot-cpp)

file(GLOB_RECURSE SOURCES "src/*.*")

add_library(FoundationSteamworks SHARED ${SOURCES})

target_include_directories(FoundationSteamworks PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/public
    ${STEAM_SDK_PATH}
)

target_link_libraries(FoundationSteamworks PRIVATE godot::cpp)

if(WIN32)
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/win64/steam_api64.lib")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/linux64/libsteam_api.so")
elseif(APPLE)
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/osx/libsteam_api.dylib")
endif()

set_target_properties(FoundationSteamworks
    PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    OUTPUT_NAME "libFoundationSteamworks.${PLATFORM}.${TYPE_STRING}.${ARCH}"
)

if(NOT WIN32)
    target_link_options(FoundationSteamworks PRIVATE "-rdynamic")
endif()

target_compile_definitions(FoundationSteamworks
    PRIVATE
    STEAM_MAJOR=${STEAM_MAJOR}
    STEAM_MINOR=${STEAM_MINOR}
)
