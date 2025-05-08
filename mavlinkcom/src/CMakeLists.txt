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

cmake_minimum_required(VERSION 3.5.0)

set(TARGET_NAME mavlinkcom)

LIST(APPEND MAVLINK_SOURCES ../common_utils/FileSystem.cpp)
LIST(APPEND MAVLINK_SOURCES ../common_utils/ThreadUtils.cpp)
LIST(APPEND MAVLINK_SOURCES AdHocConnection.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkConnection.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkFtpClient.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkLog.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkMessageBase.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkMessages.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkNode.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkTcpServer.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkVehicle.cpp)
LIST(APPEND MAVLINK_SOURCES MavLinkVideoStream.cpp)
LIST(APPEND MAVLINK_SOURCES Semaphore.cpp)
LIST(APPEND MAVLINK_SOURCES UdpSocket.cpp)
LIST(APPEND MAVLINK_SOURCES impl/AdHocConnectionImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/MavLinkConnectionImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/MavLinkFtpClientImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/MavLinkNodeImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/MavLinkTcpServerImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/MavLinkVehicleImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/MavLinkVideoStreamImpl.cpp)
LIST(APPEND MAVLINK_SOURCES impl/UdpSocketImpl.cpp)
LIST(APPEND MAVLINK_SOURCES serial_com/SerialPort.cpp)
LIST(APPEND MAVLINK_SOURCES serial_com/TcpClientPort.cpp)
LIST(APPEND MAVLINK_SOURCES serial_com/UdpClientPort.cpp)
LIST(APPEND MAVLINK_SOURCES serial_com/SocketInit.cpp)
LIST(APPEND MAVLINK_SOURCES serial_com/wifi.cpp)

IF(UNIX)
    LIST(APPEND MAVLINK_SOURCES "impl/linux/MavLinkFindSerialPorts.cpp")
ELSE()
    LIST(APPEND MAVLINK_SOURCES "impl/windows/WindowsFindSerialPorts.cpp")
ENDIF()

add_library(
    ${TARGET_NAME}
    STATIC
    ${MAVLINK_SOURCES}
)

set_target_properties(${TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ${TARGET_NAME}
    COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)

target_compile_definitions(${TARGET_NAME} PRIVATE UNICODE _UNICODE)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common_utils>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common_utils>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        # ${EIGEN_INCLUDE_DIR}
        lvmon
)

target_link_libraries(
    ${TARGET_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
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