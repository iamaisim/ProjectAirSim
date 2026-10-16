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

set(TARGET_NAME multirotor_api_gtests)
set(TARGET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(
    ${TARGET_NAME}
    EXCLUDE_FROM_ALL
    gtest_simple_flight.cpp
    gtest_jsbsim_api.cpp
    gtest_matlab_controller.cpp
)

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIMLIBS_TEST_DIR}"
)

if(WIN32)
    add_dependencies(${TARGET_NAME} multirotor_api core_sim nng-external mavlinkcom gtest_main)
elseif(APPLE)
    add_dependencies(${TARGET_NAME} multirotor_api core_sim nng-external mavlinkcom gtest_main)
else()
    add_dependencies(${TARGET_NAME} multirotor_api core_sim nng-external openssl-external mavlinkcom gtest_main)
endif()

# Set up nng and openssl for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR} ${OPENSSL_LIB_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE NNG_STATIC_LIB=ON OPENSSL_STATIC_LIB=ON)
target_compile_definitions(
    ${TARGET_NAME}
    PRIVATE
        PROJECTAIRSIM_TEST_PLUGIN_DIR="${CMAKE_SOURCE_DIR}/unreal/Blocks/Plugins/ProjectAirSim"
)
target_link_directories(${TARGET_NAME} PRIVATE ${ONNX_LIB_DIR})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TARGET_SOURCE_DIR}/include/
        ${TARGET_SOURCE_DIR}/include/../include/
        ${TARGET_SOURCE_DIR}/src/  # for private headers
        ${CMAKE_SOURCE_DIR}/core_sim/src/  # for private test helpers
        ${EIGEN_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
)

if(WIN32)
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        gtest_main
        multirotor_api
        core_sim
        nng
        mavlinkcom
        $<$<BOOL:${PROJECTAIRSIM_ENABLE_LVMON}>:lvmon>
)
else()
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        gtest_main
        multirotor_api
        core_sim
        nng
        mavlinkcom
        $<$<BOOL:${PROJECTAIRSIM_ENABLE_LVMON}>:lvmon>
        ${OPENSSL_CRYPTO_LIBRARY} # Client authorization
)
endif()

# Include CMake's GoogleTest module to use gtest_discover_tests() helper
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})

# Compile the stock wrapper independently of the MATLAB S-function interface.
set(MATLAB_WRAPPER_TARGET matlab_control_wrapper_gtests)
add_executable(
    ${MATLAB_WRAPPER_TARGET}
    EXCLUDE_FROM_ALL
    gtest_matlab_control_wrapper.cpp
    ../matlab_sfunc/matlab_control_model_wrapper.cpp
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
