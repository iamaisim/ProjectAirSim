#!/bin/bash
source /home/airsim_user/ProjectAirSim/ros/install/setup.bash

# NOTE: This requires GNU getopt.  On Mac OS X and FreeBSD, you have to install this
# separately; see below.
TEMP=$(getopt -o vd --long verbose,debug,runmode:,scenefile:,unrealoptions:,rosdomainid: \
              -n 'projectairsim_ros_entry' -- "$@")

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

# Note the quotes around '$TEMP': they are essential!
eval set -- "$TEMP"

VERBOSE=false
DEBUG=false
run_mode="airsim_node"
scenefile="scene_drone_sensors.jsonc"
unrealoptions=""
ROS_DOMAIN_ID=0

while true; do
  echo $1
  case "$1" in
    -v | --verbose ) VERBOSE=true; shift ;;
    -d | --debug ) DEBUG=true; shift ;;
    --runmode ) run_mode="$2"; shift 2 ;;
    --scenefile ) scenefile="$2"; shift 2 ;;
    --unrealoptions ) unrealoptions="$2"; shift 2 ;;
    --rosdomainid ) ROS_DOMAIN_ID="$2"; shift 2 ;;
    -- ) shift; break ;;
    * ) break ;;
  esac
done

#
# force script to be executeable
#
chmod +x /home/airsim_user/ProjectAirSim/ros/install/projectairsim_ros/lib/projectairsim_ros/projectairsim_bridge_ros2.py
chmod +x /home/airsim_user/ProjectAirSim/ros/install/projectairsim_ros/lib/projectairsim_ros/load_scene_node.py

echo "checking run_mode: $run_mode"

if [[ $run_mode = "airsim_node" ]]; then
    echo "Running airsim_node"
    export ROS_DOMAIN_ID=$ROS_DOMAIN_ID
    if [[ -n $unrealoptions ]]; then
        ros2 launch projectairsim_ros projectairsim.launch.py scenefile:=$scenefile unrealoptions:="$unrealoptions"
    else
        ros2 launch projectairsim_ros projectairsim.launch.py scenefile:=$scenefile
    fi 
elif [[ $run_mode = "px4_sitl" ]]; then
    echo "PX4 sitl iris"
    if [[ $scenefile == "scene_drone_sensors.jsonc" ]]; then
      echo "PX4 sitl iris mode default scenefile will not work with PX4, switching to scene_px4_sitl.jsonc"
      scenefile="scene_px4_sitl.jsonc"
    fi
    cd /PX4_tools/PX4-Autopilot
    make px4_sitl none_iris &
    cd /home/airsim_user
    export ROS_DOMAIN_ID=$ROS_DOMAIN_ID
    if [[ -n $unrealoptions ]]; then
        ros2 launch projectairsim_ros projectairsim.launch.py scenefile:=$scenefile enablepx4:=true unrealoptions:="$unrealoptions"
    else
        ros2 launch projectairsim_ros projectairsim.launch.py enablepx4:=true scenefile:=$scenefile
    fi 
elif [[ $run_mode = "run_tests" ]]; then
    echo "Running colcon test"
    sudo apt-get install ros-humble-launch-pytest
    source /opt/ros/humble/setup.bash
    cd /home/airsim_user/ProjectAirSim/ros
    rm -rf install log build
    colcon build
    source install/setup.bash
    colcon test --python-testing pytest --packages-select projectairsim_ros --pytest-with-coverage --ctest-args " --rerun-failed --output-on-failure"
    colcon test-result --all --verbose

    #
    # colcon test-result will create a directory with the test results
    #
    cp -r build/projectairsim_ros/test_results/projectairsim_ros/. ~/test_output
    cp -r build/projectairsim_ros/pytest_cov/. ~/test_output
else
    echo "Valid runmode values are airsim_node, px4_sitl, run_rtests"
fi
