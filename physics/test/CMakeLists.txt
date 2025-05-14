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

set(TARGET_NAME physics_gtests)
set(TARGET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(
    ${TARGET_NAME}
    gtest_physics_world.cpp
    gtest_fast_physics.cpp
    gtest_unreal_physics.cpp
)

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIMLIBS_TEST_DIR}"
)

# Add dependency to core_sim_gtests since it copies the runtime DLLs into ${SIMLIBS_TEST_DIR}
if(WIN32)
    add_dependencies(${TARGET_NAME} physics core_sim core_sim_gtests nng-external gtest_main)
else()
    add_dependencies(${TARGET_NAME} physics core_sim core_sim_gtests nng-external openssl-external gtest_main)
endif()

# Set up nng and openssl for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR} ${OPENSSL_LIB_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE NNG_STATIC_LIB=ON OPENSSL_STATIC_LIB=ON)
target_link_directories(${TARGET_NAME} PRIVATE ${ONNX_LIB_DIR})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${TARGET_SOURCE_DIR}/include/
        ${TARGET_SOURCE_DIR}/src/  # for private headers
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

if(WIN32)
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        gtest_main
        physics
        nng
        core_sim
        lvmon
)
else()
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        gtest_main
        physics
        nng
        crypto # Client authorization
        core_sim
        lvmon
)
endif()

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying [${TARGET_NAME}] test data to ${SIMLIBS_TEST_DIR}/test_data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_SOURCE_DIR}/test/test_data ${SIMLIBS_TEST_DIR}/test_data
)

# Include CMake's GoogleTest module to use gtest_discover_tests() helper
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
