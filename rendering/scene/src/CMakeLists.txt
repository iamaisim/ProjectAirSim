# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.  
# Copyright (c) 2025 IAMAI Simulations
#
# MIT License. All rights reserved.
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

set(TARGET_NAME rendering_scene)

set(INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(HEADER_FILES ${INCLUDE_DIRECTORY}/scene_global.hpp
                 ${INCLUDE_DIRECTORY}/tile_info.hpp
                 ${INCLUDE_DIRECTORY}/mesh.hpp
                 ${INCLUDE_DIRECTORY}/mesh_data_provider.hpp
                 ${INCLUDE_DIRECTORY}/gltf_data_provider.hpp
                 ${INCLUDE_DIRECTORY}/tile_manager.hpp
                 ${INCLUDE_DIRECTORY}/bing_maps_utils.hpp
                 ${INCLUDE_DIRECTORY}/tile_divider.hpp
                 ${INCLUDE_DIRECTORY}/tile_cache.hpp)

set(SRC_FILES gltf_data_provider.cpp tile_manager.cpp bing_maps_utils.cpp tile_divider.cpp tile_cache.cpp)

add_library(
    ${TARGET_NAME}
    STATIC
    ${HEADER_FILES}
    ${SRC_FILES}
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
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${TINYGLTF_INCLUDE_DIR}
)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        core_sim
        lvmon
)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [${TARGET_NAME}] build outputs to ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${TARGET_NAME}> ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../include ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/include
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CTestTestfile.cmake
)
