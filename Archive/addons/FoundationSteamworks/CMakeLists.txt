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

# Defaults to the Game-Framework git submodule vendored at <repo>/extern/Game-Framework —
# needed for Subsystem/SubsystemManager headers and to build the gameframework::shared
# import stub this addon links against at compile time. See Godot-Game-Framework's README,
# "The linking model" — same recipe already used by FoundationGameplayTags/FoundationLexicon/
# FoundationOgham — for why this MUST be the shared target (not gameframework::gameframework)
# and why the resulting libgameframework.* build byproduct here must NOT be packaged.
set(GAMEFRAMEWORK_CORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../extern/Game-Framework/Core" CACHE PATH "Path to Game-Framework's Core (engine-agnostic C++)")

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
if(NOT EXISTS "${GAMEFRAMEWORK_CORE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Game-Framework Core not found at ${GAMEFRAMEWORK_CORE_PATH}. Did you run 'git submodule update --init --recursive'?")
endif()

# Build godot-cpp as part of this build (no separate SCons pre-build step needed).
add_subdirectory(${GODOT_CPP_PATH} ${CMAKE_BINARY_DIR}/godot-cpp)

# Build Game-Framework's Core for its headers + the gameframework::shared import stub.
# OUTPUT_NAME on gameframework_shared MUST match Godot-Game-Framework's own convention
# byte-for-byte (libgameframework.<platform>.<config>.<arch>) so the DT_NEEDED/import name
# this addon embeds matches what's actually shipped at runtime.
set(GAMEFRAMEWORK_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${GAMEFRAMEWORK_CORE_PATH} ${CMAKE_BINARY_DIR}/gameframework-core)
set_target_properties(gameframework_shared PROPERTIES
    PREFIX ""
    OUTPUT_NAME "libgameframework.${PLATFORM}.${TYPE_STRING}.${ARCH}"
)

file(GLOB_RECURSE SOURCES "src/*.*")

add_library(FoundationSteamworks SHARED ${SOURCES})

target_include_directories(FoundationSteamworks PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/public
    ${STEAM_SDK_PATH}
)

target_link_libraries(FoundationSteamworks PRIVATE godot::cpp gameframework::shared)

if(WIN32)
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/win64/steam_api64.lib")
    set(STEAM_REDIST_LIB "${STEAM_SDK_PATH}/../redistributable_bin/win64/steam_api64.dll")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/linux64/libsteam_api.so")
    set(STEAM_REDIST_LIB "${STEAM_SDK_PATH}/../redistributable_bin/linux64/libsteam_api.so")
elseif(APPLE)
    target_link_libraries(FoundationSteamworks PRIVATE "${STEAM_SDK_PATH}/../redistributable_bin/osx/libsteam_api.dylib")
    set(STEAM_REDIST_LIB "${STEAM_SDK_PATH}/../redistributable_bin/osx/libsteam_api.dylib")
endif()

# Copied next to the extension itself (bin/) rather than relied upon at its
# original SDK path — the $ORIGIN/@loader_path RPATH set below only resolves
# siblings of the extension binary, and a shipped export doesn't carry
# include/sdk/ at all. Previously "worked" only by accident: CMake's default
# (non-INSTALL) RPATH baked in this build tree's absolute SDK path, which
# broke the moment BUILD_WITH_INSTALL_RPATH was set for gameframework's
# $ORIGIN resolution below (confirmed: ldd showed "libsteam_api.so => not
# found" on the first rebuild after that changed, where the old
# already-shipped binary had happened to resolve it via the stale
# build-tree RPATH).
add_custom_command(TARGET FoundationSteamworks POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${STEAM_REDIST_LIB}" "${CMAKE_CURRENT_SOURCE_DIR}/bin/"
)

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

# Avoids CMake's default absolute build-tree RPATH. This addon doesn't ship its own
# libgameframework.* (see the GAMEFRAMEWORK_CORE_PATH comment above) — the OS loader
# finds it already-loaded-by-SONAME from FoundationGameFramework.so instead, same as
# every other gem gated behind Godot-Game-Framework.
if(UNIX AND NOT APPLE)
    set_target_properties(FoundationSteamworks PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
    )
elseif(APPLE)
    set_target_properties(FoundationSteamworks PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path"
    )
endif()
