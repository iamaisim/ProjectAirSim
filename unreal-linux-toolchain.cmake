# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.  All rights reserved.
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

# Set include and link dir paths to UE 5.2.1's bundled toolchain
set(UE_TOOLCHAIN "Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v21_clang-15.0.1-centos7")
set(CMAKE_SYSROOT "$ENV{UE_ROOT}/${UE_TOOLCHAIN}/x86_64-unknown-linux-gnu")
set(CMAKE_C_COMPILER "${CMAKE_SYSROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${CMAKE_SYSROOT}/bin/clang++")
set(CMAKE_CXX_FLAGS "-I$ENV{UE_ROOT}/Engine/Source/ThirdParty/Unix/LibCxx/include -I$ENV{UE_ROOT}/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
