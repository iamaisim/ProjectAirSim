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

set(TARGET_NAME core_sim_gtests)
set(CORE_SIM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(
    ${TARGET_NAME}
    EXCLUDE_FROM_ALL
    gtest_logger.cpp
    gtest_string_utils.cpp
    gtest_transform_utils.cpp
    gtest_transform_tree.cpp
    gtest_json_utils.cpp
    gtest_simulator.cpp
    gtest_scene.cpp
    gtest_robot.cpp
    gtest_control_mapper.cpp
    gtest_env_actor.cpp
    gtest_trajectory.cpp
    gtest_link.cpp
    gtest_sensor.cpp
    gtest_camera.cpp
    gtest_camera_noise_settings.cpp
    gtest_camera_capture_settings.cpp
    gtest_imu.cpp
    gtest_imu_accelerometer.cpp
    gtest_imu_gyroscope.cpp
    gtest_airspeed.cpp
    gtest_barometer.cpp
    gtest_magnetometer.cpp
    gtest_battery.cpp
    gtest_actuator.cpp
    gtest_gimbal.cpp
    gtest_tilt.cpp
    gtest_rotor_setting.cpp
    gtest_wheel_setting.cpp
    gtest_unreal_vehicle_actuator.cpp
    gtest_noise_model_utils.cpp
    gtest_clock.cpp
    gtest_message.cpp
    gtest_image_message.cpp
    gtest_topic_manager.cpp
    gtest_battery_message.cpp
    gtest_pose_message.cpp
    gtest_kinematics_message.cpp
    gtest_rotor_message.cpp
    gtest_service_method.cpp
    gtest_lidar.cpp
    gtest_gps.cpp
    gtest_geodetic_converter.cpp
    gtest_radar.cpp
    gtest_coordinate_integration.cpp
)

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIMLIBS_TEST_DIR}"
)

if(WIN32)
    add_dependencies(${TARGET_NAME} core_sim nng-external gtest_main)
else()
    add_dependencies(${TARGET_NAME} core_sim nng-external openssl-external gtest_main)
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

if(WIN32)
target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${ONNX_INCLUDE_DIR}
        ${CORE_SIM_SOURCE_DIR}/include/
        ${CORE_SIM_SOURCE_DIR}/src/  # for private headers
)
else()
target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${CORE_SIM_SOURCE_DIR}/include/
        ${CORE_SIM_SOURCE_DIR}/src/  # for private headers
)
endif()

if(WIN32)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            gtest_main
            core_sim
            nng
            ws2_32  # req by nng on Win
            mswsock  # req by nng on Win
            advapi32  # req by nng on Win
            Bcrypt # Client authorization
            Crypt32 # Client authorization
            Ncrypt # Client authorization
            lvmon
            jsbsim
    )
else()
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            gtest_main
            core_sim
            nng
            ${OPENSSL_CRYPTO_LIBRARY} # Client authorization
            jsbsim
    )
endif()


target_include_directories(${TARGET_NAME} PRIVATE "${ONNXRUNTIME_ROOTDIR}/include")
target_link_directories(${TARGET_NAME} PRIVATE "${ONNX_LIB_DIR}")
target_link_libraries(${TARGET_NAME} PRIVATE onnxruntime jsbsim)

# Include CMake's GoogleTest module to use gtest_discover_tests() helper
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
