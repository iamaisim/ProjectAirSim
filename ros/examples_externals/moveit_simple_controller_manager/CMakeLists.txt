cmake_minimum_required(VERSION 2.8.3)
project(multidof_controller_manager)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Boost REQUIRED thread)
include_directories(SYSTEM ${Boost_INCLUDE_DIR} "../action_controller/msg_gen/cpp/include")	#WIM Changed
link_directories(${Boost_LIBRARY_DIRS})

find_package(catkin
  REQUIRED
  moveit_core
  moveit_simple_controller_manager
  pluginlib
  actionlib
  roscpp
  control_msgs
  action_controller
)

include_directories(${catkin_INCLUDE_DIRS})
link_directories(${catkin_LIBRARY_DIRS})

catkin_package(
  LIBRARIES
  INCLUDE_DIRS
  DEPENDS
    moveit_core
    action_controller
    moveit_simple_controller_manager
)

include_directories(include)

add_library(multidof_controller_manager src/moveit_simple_controller_manager.cpp)
target_link_libraries(multidof_controller_manager ${catkin_LIBRARIES} ${Boost_LIBRARIES})
add_dependencies(multidof_controller_manager ${catkin_EXPORTED_TARGETS})

install(TARGETS multidof_controller_manager LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})

install(FILES moveit_simple_controller_manager_plugin_description.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
       )
