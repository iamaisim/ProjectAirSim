# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.  
# Copyright (C) 2025 IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.
#
# Module Name:
#
#   unreal-linux-toolchain.cmake
#
# Abstract:
#
#   Basic CMake Linux toolchain file following https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-linux
#
# ---------------------------------------------------------------------------------------------------------------------

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(UE_ROOT_PATH "$ENV{UE_ROOT}")

set(UE_SDK_BASE
    "${UE_ROOT_PATH}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64"
)

if(NOT EXISTS "${UE_SDK_BASE}")
    message(FATAL_ERROR "Unreal Linux SDK path not found: ${UE_SDK_BASE}")
endif()

# Find available SDK versions (v21_*, v22_*, etc.)
file(GLOB UE_SDK_DIRS
    LIST_DIRECTORIES true
    "${UE_SDK_BASE}/*"
)

if(UE_SDK_DIRS STREQUAL "")
    message(FATAL_ERROR "No Unreal Linux SDKs found in ${UE_SDK_BASE}")
endif()

# If multiple SDKs exist, pick the last one (usually highest version)
list(SORT UE_SDK_DIRS)
list(GET UE_SDK_DIRS -1 UE_SDK_SELECTED)

get_filename_component(UE_SDK_NAME "${UE_SDK_SELECTED}" NAME)

message(STATUS "Using Unreal Linux SDK: ${UE_SDK_NAME}")

# Final toolchain path
set(CMAKE_SYSROOT
    "${UE_SDK_SELECTED}/x86_64-unknown-linux-gnu"
)

set(CMAKE_C_COMPILER   "${CMAKE_SYSROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${CMAKE_SYSROOT}/bin/clang++")
set(CMAKE_CXX_FLAGS "-I$ENV{UE_ROOT}/Engine/Source/ThirdParty/Unix/LibCxx/include -I$ENV{UE_ROOT}/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
