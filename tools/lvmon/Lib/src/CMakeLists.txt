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

set(TARGET_NAME lvmon)

LIST(APPEND LVMON_SOURCES events.cpp)
LIST(APPEND LVMON_SOURCES lvmon.cpp)
LIST(APPEND LVMON_SOURCES lvmonmessage.cpp)
LIST(APPEND LVMON_SOURCES lvmonprotocol.cpp)
LIST(APPEND LVMON_SOURCES lvmonserver.cpp)
LIST(APPEND LVMON_SOURCES serverconnectiontcp.cpp)
LIST(APPEND LVMON_SOURCES thread.cpp)

if(WIN32)
    LIST(APPEND LVMON_SOURCES serverconnectionpipe.cpp)
endif()

add_library(
    ${TARGET_NAME}
    STATIC
    ${LVMON_SOURCES}
)

set_target_properties(${TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ${TARGET_NAME}
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/LVMon
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [${TARGET_NAME}] build outputs to ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${TARGET_NAME}> ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../include ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/include
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CTestTestfile.cmake
)
endif()
