# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.  
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
#   CMake build file.
#
# ---------------------------------------------------------------------------------------------------------------------

set(TARGET_NAME physics_gtests)
set(TARGET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(
    ${TARGET_NAME}
    EXCLUDE_FROM_ALL
    gtest_physics_world.cpp
    gtest_fast_physics.cpp
    gtest_unreal_physics.cpp
    gtest_matlab_physics.cpp
    gtest_jsbsim_physics.cpp
    gtest_jsbsim_ground_callback.cpp
    ${CMAKE_SOURCE_DIR}/samples/projectairsim_runtime/ground_collision_host.cpp
)

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIMLIBS_TEST_DIR}"
)

# Add dependency to core_sim_gtests since it copies the runtime DLLs into ${SIMLIBS_TEST_DIR}
if(WIN32)
    add_dependencies(${TARGET_NAME} physics core_sim core_sim_gtests nng-external jsbsim-repo gtest_main)
elseif(APPLE)
    add_dependencies(${TARGET_NAME} physics core_sim core_sim_gtests nng-external jsbsim-repo gtest_main)
else()
    add_dependencies(${TARGET_NAME} physics core_sim core_sim_gtests nng-external jsbsim-repo openssl-external gtest_main)
endif()

# Set up nng and openssl for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR} ${OPENSSL_LIB_DIR})
target_compile_definitions(
    ${TARGET_NAME}
    PRIVATE
        NNG_STATIC_LIB=ON
        OPENSSL_STATIC_LIB=ON
        PROJECTAIRSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
target_link_directories(${TARGET_NAME} PRIVATE ${ONNX_LIB_DIR})

# Set up JSBSim for linking
target_link_directories(${TARGET_NAME} PRIVATE ${JSBSIM_LIB_DIR})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/samples/projectairsim_runtime
        ${EIGEN_INCLUDE_DIR}
        ${TARGET_SOURCE_DIR}/include/
        ${TARGET_SOURCE_DIR}/src/  # for private headers
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${JSBSIM_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/core_sim/src/  # for core_sim private test helpers
)

if(WIN32)
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        gtest_main
        physics
        nng
        core_sim
        jsbsim
        $<$<BOOL:${PROJECTAIRSIM_ENABLE_LVMON}>:lvmon>
)
else()
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        gtest_main
        physics
        nng
        ${OPENSSL_CRYPTO_LIBRARY} # Client authorization
        core_sim
        jsbsim
        $<$<BOOL:${PROJECTAIRSIM_ENABLE_LVMON}>:lvmon>
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

# Compile the stock wrapper independently of the MATLAB S-function interface.
set(MATLAB_WRAPPER_TARGET matlab_physics_wrapper_gtests)
add_executable(
    ${MATLAB_WRAPPER_TARGET}
    EXCLUDE_FROM_ALL
    gtest_matlab_physics_wrapper.cpp
    ../matlab_sfunc/matlab_physics_model_wrapper.cpp
)
set_target_properties(
    ${MATLAB_WRAPPER_TARGET}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIMLIBS_TEST_DIR}"
)
add_dependencies(${MATLAB_WRAPPER_TARGET} core_sim nng-external gtest_main)
target_compile_definitions(${MATLAB_WRAPPER_TARGET} PRIVATE NNG_STATIC_LIB=ON)
target_include_directories(
    ${MATLAB_WRAPPER_TARGET}
    PRIVATE
        ${TARGET_SOURCE_DIR}/include
        ${TARGET_SOURCE_DIR}/matlab_sfunc
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
)
target_link_directories(${MATLAB_WRAPPER_TARGET} PRIVATE ${NNG_LIB_DIR})
target_link_libraries(${MATLAB_WRAPPER_TARGET} PRIVATE gtest_main core_sim nng)
gtest_discover_tests(${MATLAB_WRAPPER_TARGET} PROPERTIES TIMEOUT 15)
