cmake_minimum_required(VERSION 3.16)
project(projectairsim_ros2_cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PROJECTAIRSIM_ROS2_CPP_ENABLE_UNREAL_TESTS
  "Build and register ROS2 C++ integration tests that require Unreal/Project AirSim and a running bridge"
  OFF)

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rosgraph_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rosidl_runtime_cpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)

# ROS2 RMW implementations load the generated type support at runtime.
# Keep interface libraries shared even though the Project AirSim C++ client
# builds its third-party transport dependencies as static libraries.
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries for ROS2 interfaces" FORCE)

set(PROJECTAIRSIM_ROS2_CPP_INTERFACES
  "msg/RadarReturn.msg"
  "msg/RadarScan.msg"
  "msg/RadarTrack.msg"
  "msg/RadarTracks.msg"
  "srv/Arm.srv"
  "srv/ArmGroup.srv"
  "srv/CancelAllRequests.srv"
  "srv/CreateVoxelGrid.srv"
  "srv/Disarm.srv"
  "srv/DisarmGroup.srv"
  "srv/GetClientInfo.srv"
  "srv/GetClock.srv"
  "srv/GetDrones.srv"
  "srv/GetOriginGeoPoint.srv"
  "srv/GetSegmentationIDByName.srv"
  "srv/GetSegmentationIDMap.srv"
  "srv/GetTopicInfo.srv"
  "srv/Land.srv"
  "srv/LandGroup.srv"
  "srv/LoadScene.srv"
  "srv/MoveOnPath.srv"
  "srv/MoveToPosition.srv"
  "srv/OccupancyGrid.srv"
  "srv/Publish.srv"
  "srv/RawRequest.srv"
  "srv/Reset.srv"
  "srv/SetObjectMaterial.srv"
  "srv/SetObjectTexture.srv"
  "srv/SetSegmentationIDByName.srv"
  "srv/SwapObjectTexture.srv"
  "srv/Takeoff.srv"
  "srv/TakeoffGroup.srv"
  "srv/Unsubscribe.srv"
  "action/MoveOnPath.action"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${PROJECTAIRSIM_ROS2_CPP_INTERFACES}
  DEPENDENCIES geometry_msgs nav_msgs std_msgs
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Project AirSim C++ client dependencies static" FORCE)

get_filename_component(PROJECTAIRSIM_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(PROJECTAIRSIM_CPP_CLIENT_DIR
    "${PROJECTAIRSIM_ROOT}/client/cpp"
    CACHE PATH "Path to the Project AirSim C++ client source tree")

if(NOT TARGET ProjectAirsimClient)
  add_subdirectory(
    "${PROJECTAIRSIM_CPP_CLIENT_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/projectairsim_cpp_client"
    EXCLUDE_FROM_ALL)
endif()

add_executable(projectairsim_ros2_cpp_node
  src/projectairsim_ros2_cpp_node.cpp
)

target_include_directories(projectairsim_ros2_cpp_node
  PRIVATE
    include
)

target_link_libraries(projectairsim_ros2_cpp_node
  ProjectAirsimClient
)

ament_target_dependencies(projectairsim_ros2_cpp_node
  geometry_msgs
  nav_msgs
  rclcpp
  rclcpp_action
  rosgraph_msgs
  sensor_msgs
  std_msgs
  tf2_ros
)

rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} "rosidl_typesupport_cpp")

target_link_libraries(projectairsim_ros2_cpp_node
  "${cpp_typesupport_target}"
)

install(TARGETS projectairsim_ros2_cpp_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(ros2_interfaces_test
    test/ros2_interfaces_test.cpp
  )
  if(TARGET ros2_interfaces_test)
    ament_target_dependencies(ros2_interfaces_test
      geometry_msgs
      nav_msgs
      rosgraph_msgs
      rosidl_runtime_cpp
      sensor_msgs
      std_msgs
      tf2_ros
    )
    target_link_libraries(ros2_interfaces_test
      "${cpp_typesupport_target}"
    )
  endif()

  ament_add_gtest(ros2_bridge_surface_regression_test
    test/ros2_bridge_surface_regression_test.cpp
  )

  ament_add_gtest(ros2_generated_interfaces_regression_test
    test/ros2_generated_interfaces_regression_test.cpp
  )
  if(TARGET ros2_generated_interfaces_regression_test)
    ament_target_dependencies(ros2_generated_interfaces_regression_test
      geometry_msgs
      rosidl_runtime_cpp
      std_msgs
    )
    target_link_libraries(ros2_generated_interfaces_regression_test
      "${cpp_typesupport_target}"
    )
  endif()

  ament_add_gtest(ros2_conversion_utils_test
    test/ros2_conversion_utils_test.cpp
  )
  if(TARGET ros2_conversion_utils_test)
    target_include_directories(ros2_conversion_utils_test
      PRIVATE
        include
    )
    ament_target_dependencies(ros2_conversion_utils_test
      geometry_msgs
      sensor_msgs
      rosidl_runtime_cpp
    )
    target_link_libraries(ros2_conversion_utils_test
      ProjectAirsimClient
      "${cpp_typesupport_target}"
    )
  endif()

  if(PROJECTAIRSIM_ROS2_CPP_ENABLE_UNREAL_TESTS)
    ament_add_gtest(ros2_unreal_integration_test
      test/ros2_unreal_integration_test.cpp
      TIMEOUT 900
    )
    if(TARGET ros2_unreal_integration_test)
      ament_target_dependencies(ros2_unreal_integration_test
        geometry_msgs
        rclcpp
        rosgraph_msgs
        sensor_msgs
      )
      target_link_libraries(ros2_unreal_integration_test
        "${cpp_typesupport_target}"
      )
      set_tests_properties(ros2_unreal_integration_test
        PROPERTIES
          LABELS "unreal;integration"
      )
    endif()
  endif()
endif()

ament_export_include_directories(include)
ament_package()
