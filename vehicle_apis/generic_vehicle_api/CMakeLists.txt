# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) 2025 IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.
#
# Module Name:
#
#   CMakeLists.txt
#
# Abstract:
#
#   CMake build file for the generic vehicle API library.
#
# ---------------------------------------------------------------------------------------------------------------------

set(TARGET_NAME generic_vehicle_api)

add_library(
    ${TARGET_NAME}
    STATIC
    src/unreal_vehicle_api.cpp
)

set_target_properties(${TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ${TARGET_NAME}
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)

add_dependencies(${TARGET_NAME} core_sim)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${EIGEN_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        core_sim
)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [${TARGET_NAME}] build outputs to ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${TARGET_NAME}> ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/include
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CTestTestfile.cmake
)
