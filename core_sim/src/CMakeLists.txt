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

set(TARGET_NAME core_sim)

add_library(
    ${TARGET_NAME}
    STATIC
    logger.cpp
    string_utils.cpp
    math_utils.cpp
    geodetic_converter.cpp
    json_utils.cpp
    message/airspeed_message.cpp
    message/barometer_message.cpp
    message/camera_info_message.cpp
    message/collision_info_message.cpp
    message/distance_sensor_message.cpp
    message/flight_control_setpoint_message.cpp
    message/flight_control_rc_input_message.cpp
    message/float_message.cpp
    message/gps_message.cpp
    message/image_message.cpp
    message/imu_message.cpp
    message/int_list_message.cpp
    message/battery_message.cpp
    message/int8_message.cpp
    message/int32_message.cpp
    message/joint_state_message.cpp
    message/kinematics_message.cpp
    message/lidar_message.cpp
    message/magnetometer_message.cpp
    message/message.cpp
    message/message_utils.cpp
    message/pose_stamped_message.cpp
    message/pose_message.cpp
    message/radar_detection_message.cpp
    message/radar_track_message.cpp
    message/ready_state_message.cpp
    message/request_message.cpp
    message/response_success_message.cpp
    message/response_failure_message.cpp
    message/rotor_message.cpp
    message/transform_message.cpp
    message/physics_state_message.cpp
    topic_manager.cpp
    topic.cpp
    simulator.cpp
    scene.cpp
    actor.cpp
    actor/robot.cpp
    actor/env_actor.cpp
    actor/env_object.cpp
    actor/env_actor_grounded.cpp
    actor/env_car.cpp
    trajectory.cpp
    link.cpp
    link/inertial.cpp
    link/collision.cpp
    link/visual.cpp
    link/material.cpp
    link/geometry.cpp
    link/geometry/file_mesh.cpp
    link/geometry/unreal_mesh.cpp
    link/geometry/skeletal_mesh.cpp
    joint.cpp
    sensors/airspeed.cpp
    sensors/sensor.cpp
    sensors/camera.cpp
    sensors/distance_sensor.cpp
    sensors/imu.cpp
    sensors/barometer.cpp
    sensors/magnetometer.cpp
    sensors/lidar.cpp
    sensors/gps.cpp
    sensors/radar.cpp
    sensors/battery.cpp
    client_authorization.cpp
    clock.cpp
    actuators/actuator.cpp
    actuators/control_mapper.cpp
    actuators/gimbal.cpp
    actuators/lift_drag_control_surface.cpp
    actuators/rotor.cpp
    actuators/wheel.cpp
    actuators/tilt.cpp
    transforms/transform_utils.cpp
    transforms/transform_tree.cpp
    viewport_camera.cpp
    service.cpp
    service_manager.cpp
    state_manager.cpp
)

set_target_properties(${TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ${TARGET_NAME}
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)

if(WIN32)
    add_dependencies(${TARGET_NAME} nng-external)
else()
    add_dependencies(${TARGET_NAME} nng-external openssl-external)
endif()

# Set up nng and openssl for linking
target_link_directories(${TARGET_NAME} PRIVATE ${NNG_LIB_DIR})
target_compile_definitions(${TARGET_NAME} PRIVATE NNG_STATIC_LIB=ON)

target_include_directories(${TARGET_NAME} PRIVATE "${ONNXRUNTIME_ROOTDIR}/include")
target_link_directories(${TARGET_NAME} PRIVATE "${ONNXRUNTIME_ROOTDIR}/lib")
target_link_libraries(${TARGET_NAME} PRIVATE onnxruntime)


target_include_directories(
    ${TARGET_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JSON_INCLUDE_DIR}
        ${NNG_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

if(WIN32)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            nng
            ws2_32  # req by nng on Win
            mswsock  # req by nng on Win
            advapi32  # req by nng on Win
            Bcrypt # Client authorization
            Crypt32 # Client authorization
            Ncrypt # Client authorization
            lvmon
    )
else()
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            nng
            crypto # Client authorization
    )
endif()

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [${TARGET_NAME}] build outputs to ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${TARGET_NAME}> ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../include ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/include
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove -f ${UE_PLUGIN_SIMLIBS_DIR}/${TARGET_NAME}/$<IF:$<CONFIG:Release>,Release,Debug>/CTestTestfile.cmake
)
