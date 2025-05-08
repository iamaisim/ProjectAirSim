cmake_minimum_required(VERSION 2.8.3)
project(action_controller)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Boost REQUIRED thread)

include_directories(SYSTEM "msg_gen/cpp/include")

find_package(
  catkin
  REQUIRED
  moveit_core
  pluginlib
  actionlib
  roscpp
  trajectory_msgs
  actionlib_msgs
  std_msgs
  message_generation
  genmsg
)

include_directories(${catkin_INCLUDE_DIRS})
link_directories(${catkin_LIBRARY_DIRS})

add_action_files(
  DIRECTORY action
  FILES MultiDofFollowJointTrajectory.action
)

generate_messages(
  DEPENDENCIES
  trajectory_msgs
  actionlib_msgs
)

catkin_package(
  LIBRARIES
  INCLUDE_DIRS
  DEPENDS
  CATKIN_DEPENDS
  actionlib_msgs
  moveit_core
)

include_directories(
  include
)

# add_library(action_controller src/actionController.cpp)
add_executable(action_controller src/actionController.cpp)
target_link_libraries(action_controller ${catkin_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(action_controller ${action_controller_EXPORTED_TARGETS})


#install(TARGETS action_controller LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})


