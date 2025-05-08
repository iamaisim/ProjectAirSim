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

set(TARGET_NAME multirotor_api_gtests)
set(TARGET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(
    ${TARGET_NAME}
    gtest_simple_flight.cpp
)

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIMLIBS_TEST_DIR}"
)

# Add dependency to core_sim_gtests since it copies the runtime DLLs into ${SIMLIBS_TEST_DIR}
if(WIN32)
    add_dependencies(${TARGET_NAME} multirotor_api core_sim core_sim_gtests nng-external mavlinkcom gtest_main)
else()
    add_dependencies(${TARGET_NAME} multirotor_api core_sim core_sim_gtests nng-external openssl-external mavlinkcom gtest_main)
endif()

# Set up nng and openssl for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR} ${OPENSSL_LIB_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE NNG_STATIC_LIB=ON OPENSSL_STATIC_LIB=ON)
target_link_directories(${TARGET_NAME} PRIVATE ${ONNX_LIB_DIR})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TARGET_SOURCE_DIR}/include/
        ${TARGET_SOURCE_DIR}/include/../include/
        ${TARGET_SOURCE_DIR}/src/  # for private headers
        ${EIGEN_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
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
        lvmon
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
        lvmon
        crypto # Client authorization
)
endif()

# Include CMake's GoogleTest module to use gtest_discover_tests() helper
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
