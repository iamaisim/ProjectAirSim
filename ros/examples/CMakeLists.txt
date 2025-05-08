cmake_minimum_required(VERSION 2.8.12)

find_package(catkin REQUIRED COMPONENTS roslaunch)

add_subdirectory(build_map)
add_subdirectory(navigate_map)
