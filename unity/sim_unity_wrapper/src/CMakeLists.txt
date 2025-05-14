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

set(TARGET_NAME sim_unity_wrapper)

add_library(
    ${TARGET_NAME}
    SHARED
    sim_unity_wrapper.cpp
)

if(NOT WIN32)
    set(DEP_OPENSSL openssl-external)
endif()
add_dependencies(
    ${TARGET_NAME}
    simserver
    core_sim
    nng-external
    ${DEP_OPENSSL}
    physics
    multirotor_api
    rendering_scene
)

# Set up nng for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR} ${OPENSSL_LIB_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE NNG_STATIC_LIB=ON)
target_link_directories(${TARGET_NAME} PRIVATE ${ONNX_LIB_DIR})


target_include_directories(
    ${TARGET_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

if(WIN32)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            simserver
            core_sim
            physics
            mavlinkcom
            multirotor_api
            rendering_scene
            nng
            ws2_32  # req by nng on Win
            mswsock  # req by nng on Win
            advapi32  # req by nng on Win
            lvmon
    )
else()
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        simserver
        core_sim
        physics
        mavlinkcom
        multirotor_api
        rendering_scene
        nng
    )
endif()

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [${TARGET_NAME}] build outputs to ${UNITY_WRAPPER_DLL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_NAME:${TARGET_NAME}> ${UNITY_WRAPPER_DLL_DIR}
)
