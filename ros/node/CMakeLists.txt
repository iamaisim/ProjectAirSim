cmake_minimum_required(VERSION 3.5)
project(projectairsim_ros)


find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2_sensor_msgs REQUIRED)
#find_package(projectairsim_interfaces)

find_package(launch_testing_ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

install(DIRECTORY config/
  DESTINATION share/${PROJECT_NAME}/params)

#
#  Set CMake variables from info in package.xml.
#
#  pytest coverage will not be enabled if this is not done
#
ament_package_xml()

install(DIRECTORY launch rviz DESTINATION share/${PROJECT_NAME})

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/GetOriginGeoPoint.srv"
  "srv/MoveToPosition.srv"
  "srv/GetColorFromMeshId.srv"
  "srv/GetColorFromSegId.srv"
  "srv/GetSegIdFromMeshId.srv"
  "srv/SetGPSPosition.srv"
  "srv/GetClock.srv"
  "srv/GetSegIdFromColor.srv"
  "srv/CreateVoxelGrid.srv"
  "srv/OccupancyGrid.srv"
  "srv/Land.srv"
  "srv/SetLocalPosition.srv"
  "srv/Reset.srv"
  "srv/LandGroup.srv"
  "srv/Takeoff.srv"
  "srv/Arm.srv"
  "srv/ArmGroup.srv"
  "srv/Disarm.srv"
  "srv/DisarmGroup.srv"
  "srv/GetMeshIdsFromSegId.srv"
  "srv/TakeoffGroup.srv"
  "srv/MoveOnPath.srv"
  "srv/LoadScene.srv"
  "srv/GetLoggingDirectory.srv"
  "action/MoveOnPath.action"
  "msg/Kinematics.msg"
  "msg/Waypoint.msg"
  "msg/CollisionInfo.msg" DEPENDENCIES std_msgs geometry_msgs nav_msgs
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

install(PROGRAMS  
  scripts/ros2/projectairsim_bridge_ros2.py
  scripts/ros2/webrtc_node.py
  scripts/ros2/load_scene_node.py   
  DESTINATION lib/${PROJECT_NAME} )


if(BUILD_TESTING)
  
  #
  #  Run pytest tests via ros2 launch_testing launch_pytest package (https://github.com/ros2/launch)
  #
  find_package(ament_cmake_pytest REQUIRED)
  
  set(_pytests_tests
  test/python/pytest_fixture.py
  test/python/test_image_topics.py
  test/python/test_drone_spawn_and_position.py
  test/python/test_env_actor_spawn_position.py
  test/python/test_odometry_topic.py
  test/python/test_standard_topics.py
  test/python/test_two_drones.py
  )
  foreach(_test_path ${_pytests_tests})
    get_filename_component(_test_name ${_test_path} NAME_WE)
    ament_add_pytest_test(${_test_name} ${_test_path}
      TIMEOUT 540
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
  endforeach()

endif()

ament_package()
