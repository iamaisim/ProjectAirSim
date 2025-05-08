# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.  All rights reserved.
#
# Module Name:
#
#   CMakeLists.txt
#
# Abstract:
#
#   CMake build file.
#
# ---------------------------------------------------------------------------------------------------------------------

# Add main mavlinkcom source
add_subdirectory(src)

# Add mavlinkcom unit tests
# (This is currently disabled for Linux because it uses std::filesystem and the version of
# libc++ that comes in Unreal's packaged Linux toolchain doesn't seem to support this)
if(WIN32)
    add_subdirectory("test")
endif()