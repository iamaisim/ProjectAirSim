cmake_minimum_required(VERSION 2.8.12)

find_package(catkin REQUIRED COMPONENTS roslaunch)

roslaunch_add_file_check(navigate_map.launch)
