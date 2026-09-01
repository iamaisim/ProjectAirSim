cmake_minimum_required(VERSION 3.20)

set(PROJECTAIRSIM_CPP_CLIENT_VERSION 1.0.0)
project(ProjectAirSimCppClient VERSION ${PROJECTAIRSIM_CPP_CLIENT_VERSION} LANGUAGES CXX)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

get_filename_component(PROJECTAIRSIM_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(NOT WIN32)
    find_package(Threads REQUIRED)
endif()

FetchContent_Declare(
    nlohmann-json
    URL                  https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp
    URL_HASH             SHA256=9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6
    DOWNLOAD_NO_EXTRACT  TRUE
)
FetchContent_GetProperties(nlohmann-json)
if(NOT nlohmann-json_POPULATED)
    FetchContent_Populate(nlohmann-json)
endif()
set(JSON_INCLUDE_DIR "${nlohmann-json_SOURCE_DIR}")

FetchContent_Declare(
    msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG        cpp-6.1.0
)
FetchContent_GetProperties(msgpack)
if(NOT msgpack_POPULATED)
    FetchContent_Populate(msgpack)
endif()
set(MSGPACK_INCLUDE_DIR "${msgpack_SOURCE_DIR}/include")
add_compile_definitions(MSGPACK_NO_BOOST)

find_package(Eigen3 QUIET)
if(NOT Eigen3_FOUND)
    if(EXISTS "/usr/include/eigen3/Eigen/Core")
        set(EIGEN_INCLUDE_DIR "/usr/include/eigen3")
    elseif(EXISTS "/usr/local/include/eigen3/Eigen/Core")
        set(EIGEN_INCLUDE_DIR "/usr/local/include/eigen3")
    endif()
    if(NOT EIGEN_INCLUDE_DIR)
        FetchContent_Declare(
            Eigen3
            GIT_REPOSITORY https://github.com/eigen-mirror/eigen.git
            GIT_TAG        3.4.0
        )
        FetchContent_GetProperties(Eigen3)
        if(NOT eigen3_POPULATED)
            FetchContent_Populate(Eigen3)
        endif()
        set(EIGEN_INCLUDE_DIR "${eigen3_SOURCE_DIR}")
    endif()
endif()
if(Eigen3_FOUND)
    get_target_property(PROJECTAIRSIM_EIGEN_INCLUDE_DIR Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
else()
    set(PROJECTAIRSIM_EIGEN_INCLUDE_DIR "${EIGEN_INCLUDE_DIR}")
endif()

set(NNG_TESTS OFF CACHE BOOL "" FORCE)
set(NNG_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    nng
    GIT_REPOSITORY https://github.com/nanomsg/nng.git
    GIT_TAG        v1.11
)
FetchContent_GetProperties(nng)
if(NOT nng_POPULATED)
    FetchContent_Populate(nng)
endif()
add_subdirectory(${nng_SOURCE_DIR} ${nng_BINARY_DIR} EXCLUDE_FROM_ALL)

add_library(NNGI STATIC
    NNGI/NNGI.cpp
)

target_include_directories(NNGI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/NNGI>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/NNGI/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/NNGI/include/NNGI>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NNGI>
)

target_link_libraries(NNGI
    PUBLIC
        nng
)

target_compile_definitions(NNGI
    PUBLIC
        NNGI_STATIC_LIB
        NNG_STATIC_LIB
    PRIVATE
        NNGI_EXPORTS
)

add_library(ProjectAirSimMessageLib STATIC
    ProjectAirSimMessageLib/src/image_message.cpp
    ProjectAirSimMessageLib/src/jsonc.cpp
    ProjectAirSimMessageLib/src/message.cpp
    ProjectAirSimMessageLib/src/pose_stamped_message.cpp
    ProjectAirSimMessageLib/src/request_message.cpp
    ProjectAirSimMessageLib/src/response_failure_message.cpp
    ProjectAirSimMessageLib/src/response_message.cpp
    ProjectAirSimMessageLib/src/response_success_message.cpp
)

target_include_directories(ProjectAirSimMessageLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirSimMessageLib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirSimMessageLib/include/ProjectAirSimMessage>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirSimMessageLib/include>
        $<BUILD_INTERFACE:${PROJECTAIRSIM_ROOT}>
        $<BUILD_INTERFACE:${PROJECTAIRSIM_ROOT}/core_sim/include>
        $<BUILD_INTERFACE:${PROJECTAIRSIM_ROOT}/core_sim/src>
        $<BUILD_INTERFACE:${JSON_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${MSGPACK_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ProjectAirSimMessage>
)

if(Eigen3_FOUND)
    target_link_libraries(ProjectAirSimMessageLib PUBLIC Eigen3::Eigen)
else()
    target_include_directories(ProjectAirSimMessageLib PUBLIC ${EIGEN_INCLUDE_DIR})
endif()

add_library(ProjectAirsimClient STATIC
    ProjectAirsimClientLib/src/ProjectAirsimClient.cpp
    ProjectAirsimClientLib/src/AsyncResult.cpp
    ProjectAirsimClientLib/src/AsyncResultInternal.cpp
    ProjectAirsimClientLib/src/Client.cpp
    ProjectAirsimClientLib/src/Drone.cpp
    ProjectAirsimClientLib/src/ICommChannel.cpp
    ProjectAirsimClientLib/src/Log.cpp
    ProjectAirsimClientLib/src/Message.cpp
    ProjectAirsimClientLib/src/RefCounted.cpp
    ProjectAirsimClientLib/src/Status.cpp
    ProjectAirsimClientLib/src/Topics.cpp
    ProjectAirsimClientLib/src/Types.cpp
    ProjectAirsimClientLib/src/Utils.cpp
    ProjectAirsimClientLib/src/World.cpp
        ProjectAirsimClientLib/src/Rover.cpp
        ProjectAirsimClientLib/src/EnvActor.cpp
        ProjectAirsimClientLib/src/StaticSensorActor.cpp
        ${PROJECTAIRSIM_ROOT}/core_sim/src/math_utils.cpp
)

target_include_directories(ProjectAirsimClient
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirsimClientLib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirsimClientLib/Include/ProjectAirsimClient>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirsimClientLib/Include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirSimMessageLib/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/NNGI/include>
        $<BUILD_INTERFACE:${PROJECTAIRSIM_ROOT}>
        $<BUILD_INTERFACE:${PROJECTAIRSIM_ROOT}/core_sim/include>
        $<BUILD_INTERFACE:${PROJECTAIRSIM_ROOT}/core_sim/src>
        $<BUILD_INTERFACE:${JSON_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${MSGPACK_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ProjectAirsimClient>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ProjectAirSimMessage>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NNGI>
)

if(Eigen3_FOUND)
    target_link_libraries(ProjectAirsimClient PUBLIC Eigen3::Eigen)
else()
    target_include_directories(ProjectAirsimClient PUBLIC ${EIGEN_INCLUDE_DIR})
endif()

target_link_libraries(ProjectAirsimClient
    PUBLIC
        ProjectAirSimMessageLib
        NNGI
        nng
)

if(WIN32)
    target_link_libraries(ProjectAirsimClient
        PUBLIC
            Version
    )
endif()

target_compile_definitions(ProjectAirsimClient
    PUBLIC
        PROJECTAIRSIMCLIENT_STATIC
        NNGI_STATIC_LIB
        NNG_STATIC_LIB
    PRIVATE
        PROJECTAIRSIMCLIENT_LIB_BUILD
        MSGPACK_NO_BOOST
)

if(WIN32)
    set(PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB
        "${CMAKE_CURRENT_BINARY_DIR}/ProjectAirSimCppClient.lib"
    )
    add_custom_command(
        OUTPUT ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
        COMMAND ${CMAKE_AR} /NOLOGO /OUT:${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
                $<TARGET_FILE:ProjectAirsimClient>
                $<TARGET_FILE:ProjectAirSimMessageLib>
                $<TARGET_FILE:NNGI>
                $<TARGET_FILE:nng>
        DEPENDS ProjectAirsimClient ProjectAirSimMessageLib NNGI nng
        COMMENT "Creating merged ProjectAirSimCppClient static library"
        VERBATIM
    )
else()
    set(PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB
        "${CMAKE_CURRENT_BINARY_DIR}/libProjectAirSimCppClient.a"
    )
    set(PROJECTAIRSIM_CPP_CLIENT_MRI_SCRIPT
        "${CMAKE_CURRENT_BINARY_DIR}/ProjectAirSimCppClient.mri"
    )
    file(GENERATE
        OUTPUT ${PROJECTAIRSIM_CPP_CLIENT_MRI_SCRIPT}
        CONTENT
"CREATE ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
ADDLIB $<TARGET_FILE:ProjectAirsimClient>
ADDLIB $<TARGET_FILE:ProjectAirSimMessageLib>
ADDLIB $<TARGET_FILE:NNGI>
ADDLIB $<TARGET_FILE:nng>
SAVE
END
"
    )
    add_custom_command(
        OUTPUT ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
        COMMAND /bin/sh -c "\"$1\" -M < \"$2\"" sh ${CMAKE_AR} ${PROJECTAIRSIM_CPP_CLIENT_MRI_SCRIPT}
        DEPENDS ProjectAirsimClient ProjectAirSimMessageLib NNGI nng ${PROJECTAIRSIM_CPP_CLIENT_MRI_SCRIPT}
        COMMENT "Creating merged ProjectAirSimCppClient static library"
        VERBATIM
    )
endif()

add_custom_target(ProjectAirSimCppClientMerged ALL
    DEPENDS ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
)

add_library(FakeNNGI STATIC
    tests/FakeNNGI.cpp
)

target_compile_definitions(FakeNNGI
    PUBLIC
        NNGI_STATIC_LIB
)

target_include_directories(FakeNNGI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/NNGI/include
        ${CMAKE_CURRENT_SOURCE_DIR}/NNGI/include/NNGI
)

add_library(ProjectAirsimClientMocked STATIC
    ProjectAirsimClientLib/src/ProjectAirsimClient.cpp
    ProjectAirsimClientLib/src/AsyncResult.cpp
    ProjectAirsimClientLib/src/AsyncResultInternal.cpp
    ProjectAirsimClientLib/src/Client.cpp
    ProjectAirsimClientLib/src/Drone.cpp
    ProjectAirsimClientLib/src/ICommChannel.cpp
    ProjectAirsimClientLib/src/Log.cpp
    ProjectAirsimClientLib/src/Message.cpp
    ProjectAirsimClientLib/src/RefCounted.cpp
    ProjectAirsimClientLib/src/Status.cpp
    ProjectAirsimClientLib/src/Topics.cpp
    ProjectAirsimClientLib/src/Types.cpp
    ProjectAirsimClientLib/src/Utils.cpp
    ProjectAirsimClientLib/src/World.cpp
    ProjectAirsimClientLib/src/Rover.cpp
    ProjectAirsimClientLib/src/EnvActor.cpp
    ProjectAirsimClientLib/src/StaticSensorActor.cpp
    ${PROJECTAIRSIM_ROOT}/core_sim/src/math_utils.cpp
)

target_include_directories(ProjectAirsimClientMocked
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirsimClientLib
        ${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirsimClientLib/Include/ProjectAirsimClient
        ${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirsimClientLib/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/ProjectAirSimMessageLib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/NNGI/include
        ${PROJECTAIRSIM_ROOT}
        ${PROJECTAIRSIM_ROOT}/core_sim/include
        ${PROJECTAIRSIM_ROOT}/core_sim/src
        ${JSON_INCLUDE_DIR}
        ${MSGPACK_INCLUDE_DIR}
)

if(Eigen3_FOUND)
    target_link_libraries(ProjectAirsimClientMocked PUBLIC Eigen3::Eigen)
else()
    target_include_directories(ProjectAirsimClientMocked PUBLIC ${EIGEN_INCLUDE_DIR})
endif()

target_link_libraries(ProjectAirsimClientMocked
    PUBLIC
        ProjectAirSimMessageLib
        FakeNNGI
)

if(WIN32)
    target_link_libraries(ProjectAirsimClientMocked
        PUBLIC
            Version
    )
endif()

target_compile_definitions(ProjectAirsimClientMocked
    PUBLIC
        PROJECTAIRSIMCLIENT_STATIC
        NNGI_STATIC_LIB
        NNG_STATIC_LIB
    PRIVATE
        PROJECTAIRSIMCLIENT_LIB_BUILD
        MSGPACK_NO_BOOST
)

add_executable(hello_drone
    example_user_apps/HelloDrone/HelloDrone.cpp
)

add_executable(cpp_client_scenarios
    example_user_apps/CppClientScenarios/CppClientScenarios.cpp
)

add_executable(user_scenario_template
    example_user_apps/UserScenarioTemplate/UserScenarioTemplate.cpp
)

add_executable(user_rover_scenario
    example_user_apps/UserRoverScenario/UserRoverScenario.cpp
)

add_executable(user_env_actor_scenario
    example_user_apps/UserEnvActorScenario/UserEnvActorScenario.cpp
)

add_executable(user_static_sensor_scenario
    example_user_apps/UserStaticSensorScenario/UserStaticSensorScenario.cpp
)

add_executable(user_lidar_scenario
    example_user_apps/UserLidarScenario/UserLidarScenario.cpp
)

add_executable(user_radar_scenario
    example_user_apps/UserRadarScenario/UserRadarScenario.cpp
)

add_executable(cpp_client_mocked_unit_tests
    tests/CppClientUnitTests.cpp
)

add_executable(cpp_client_unit_tests
    tests/CppClientIntegrationTests.cpp
)

include(CTest)
if(BUILD_TESTING AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_test(NAME cpp_client_mocked_unit_tests COMMAND cpp_client_mocked_unit_tests)
endif()

target_link_libraries(hello_drone
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(cpp_client_scenarios
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(user_scenario_template
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(user_rover_scenario
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(user_env_actor_scenario
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(user_static_sensor_scenario
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(user_lidar_scenario
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(user_radar_scenario
    PRIVATE
        ProjectAirsimClient
)

target_link_libraries(cpp_client_mocked_unit_tests
    PRIVATE
        ProjectAirsimClientMocked
)

target_link_libraries(cpp_client_unit_tests
    PRIVATE
        ProjectAirsimClient
)

if(NOT WIN32)
    target_link_libraries(hello_drone PRIVATE Threads::Threads)
    target_link_libraries(cpp_client_scenarios PRIVATE Threads::Threads)
    target_link_libraries(user_scenario_template PRIVATE Threads::Threads)
    target_link_libraries(user_rover_scenario PRIVATE Threads::Threads)
    target_link_libraries(user_env_actor_scenario PRIVATE Threads::Threads)
    target_link_libraries(user_static_sensor_scenario PRIVATE Threads::Threads)
    target_link_libraries(user_lidar_scenario PRIVATE Threads::Threads)
    target_link_libraries(user_radar_scenario PRIVATE Threads::Threads)
    target_link_libraries(ProjectAirsimClientMocked PUBLIC Threads::Threads)
    target_link_libraries(cpp_client_mocked_unit_tests PRIVATE Threads::Threads)
    target_link_libraries(cpp_client_unit_tests PRIVATE Threads::Threads)
endif()

# ---------------------------------------------------------------------------------------------------------------------
#
# Install and package support.
#
# ---------------------------------------------------------------------------------------------------------------------

set(PROJECTAIRSIM_CPP_CLIENT_EXAMPLE_TARGETS
    hello_drone
    cpp_client_scenarios
    user_scenario_template
    user_rover_scenario
    user_env_actor_scenario
    user_static_sensor_scenario
    user_lidar_scenario
    user_radar_scenario
)

install(FILES ${PROJECTAIRSIM_CPP_CLIENT_FINAL_LIB}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS ${PROJECTAIRSIM_CPP_CLIENT_EXAMPLE_TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ProjectAirsimClientLib/Include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ProjectAirSimMessageLib/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY NNGI/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${PROJECTAIRSIM_ROOT}/core_sim/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(FILES ${JSON_INCLUDE_DIR}/json.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${MSGPACK_INCLUDE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.ipp"
)

if(PROJECTAIRSIM_EIGEN_INCLUDE_DIR AND EXISTS "${PROJECTAIRSIM_EIGEN_INCLUDE_DIR}/Eigen")
    install(DIRECTORY ${PROJECTAIRSIM_EIGEN_INCLUDE_DIR}/Eigen
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

install(DIRECTORY example_user_apps/
    DESTINATION examples
    FILES_MATCHING
        PATTERN "*.cpp"
        PATTERN "*.md"
        PATTERN "*.txt"
        PATTERN "*.sln" EXCLUDE
        PATTERN "*.vcxproj" EXCLUDE
        PATTERN "*.filters" EXCLUDE
)

install(DIRECTORY scripts/
    DESTINATION scripts
    FILES_MATCHING PATTERN "*.sh"
)

install(DIRECTORY ${PROJECTAIRSIM_ROOT}/client/python/example_user_scripts/sim_config/
    DESTINATION examples/sim_config
    FILES_MATCHING
        PATTERN "*.json"
        PATTERN "*.jsonc"
)

if(WIN32)
    set(PROJECTAIRSIM_CPP_CLIENT_PACKAGE_PLATFORM "win64")
    set(PROJECTAIRSIM_CPP_CLIENT_PACKAGE_GENERATOR "ZIP")
    set(PROJECTAIRSIM_CPP_CLIENT_LIB_PREFIX "")
    set(PROJECTAIRSIM_CPP_CLIENT_LIB_SUFFIX ".lib")
else()
    set(PROJECTAIRSIM_CPP_CLIENT_PACKAGE_PLATFORM "linux64")
    set(PROJECTAIRSIM_CPP_CLIENT_PACKAGE_GENERATOR "TGZ")
    set(PROJECTAIRSIM_CPP_CLIENT_LIB_PREFIX "lib")
    set(PROJECTAIRSIM_CPP_CLIENT_LIB_SUFFIX ".a")
endif()

set(PROJECTAIRSIM_CPP_CLIENT_PACKAGE_NAME
    "ProjectAirSimCppClient-${PROJECT_VERSION}-${PROJECTAIRSIM_CPP_CLIENT_PACKAGE_PLATFORM}"
)

configure_package_config_file(
    packaging/ProjectAirSimCppClientConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ProjectAirSimCppClientConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ProjectAirSimCppClient
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ProjectAirSimCppClientConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file(
    packaging/README.md.in
    ${CMAKE_CURRENT_BINARY_DIR}/README.md
    @ONLY
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ProjectAirSimCppClientConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ProjectAirSimCppClientConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ProjectAirSimCppClient
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/README.md
        ${PROJECTAIRSIM_ROOT}/docs/license.md
    DESTINATION .
)

set(CPACK_PACKAGE_NAME ProjectAirSimCppClient)
set(CPACK_PACKAGE_VENDOR "IAMAI CONSULTING CORP")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME ${PROJECTAIRSIM_CPP_CLIENT_PACKAGE_NAME})
set(CPACK_PACKAGE_DIRECTORY "${PROJECTAIRSIM_ROOT}/packages/projectairsim_cpp_client")
set(CPACK_GENERATOR ${PROJECTAIRSIM_CPP_CLIENT_PACKAGE_GENERATOR})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_PACKAGE_CHECKSUM SHA256)
include(CPack)
