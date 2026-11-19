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

set(TARGET_NAME projectairsim-runtime)

add_executable(
    ${TARGET_NAME}
    ground_collision_host.cpp
    main.cpp
)

if(WIN32)
    add_dependencies(
        ${TARGET_NAME}
        simserver
        core_sim
        nng-external
        physics
        multirotor_api
        rendering_scene
    )
elseif(APPLE)
    add_dependencies(
        ${TARGET_NAME}
        simserver
        core_sim
        nng-external
        physics
        multirotor_api
        rendering_scene
    )
else()
    add_dependencies(
        ${TARGET_NAME}
        simserver
        core_sim
        nng-external
        openssl-external
        physics
        multirotor_api
        rendering_scene
    )
endif()

# Set up nng and openssl for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR} ${OPENSSL_LIB_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE NNG_STATIC_LIB=ON OPENSSL_STATIC_LIB=ON)
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
            $<$<BOOL:${PROJECTAIRSIM_ENABLE_LVMON}>:lvmon>
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
        ${OPENSSL_CRYPTO_LIBRARY} # Client authorization
)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE dl) # Required by static libcrypto
endif()

# Keep the standalone runtime self-contained. Both core_sim and physics use
# shared third-party libraries, so launching the executable directly from its
# documented output directory must not depend on a developer shell or PATH.
if(WIN32)
    set(RUNTIME_ONNX_SHARED_LIB "${ONNX_LIB_DIR}/onnxruntime.dll")
elseif(APPLE)
    set(RUNTIME_ONNX_SHARED_LIBS "${ONNXRUNTIME_LIBRARY}")
    if(DEFINED ONNXRUNTIME_ALIAS_LIBRARY AND NOT ONNXRUNTIME_ALIAS_LIBRARY STREQUAL "${ONNXRUNTIME_LIBRARY}")
        list(APPEND RUNTIME_ONNX_SHARED_LIBS "${ONNXRUNTIME_ALIAS_LIBRARY}")
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES BUILD_RPATH "@loader_path")
else()
    set(RUNTIME_ONNX_SHARED_LIB "${ONNXRUNTIME_LIBRARY}")
    set_target_properties(${TARGET_NAME} PROPERTIES BUILD_RPATH "$ORIGIN")
endif()

add_custom_command(
    TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${JSBSIM_SHARED_LIB}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
    COMMENT "Copying Project AirSim Runtime shared-library dependencies"
)

if(APPLE)
    foreach(RUNTIME_ONNX_SHARED_LIB IN LISTS RUNTIME_ONNX_SHARED_LIBS)
        add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RUNTIME_ONNX_SHARED_LIB}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        )
    endforeach()
else()
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RUNTIME_ONNX_SHARED_LIB}"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"
    )
endif()
