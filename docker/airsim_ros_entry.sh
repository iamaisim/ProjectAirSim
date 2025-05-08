#!/bin/bash
source /home/airsim_user/ProjectAirSim/ros/node/install/setup.bash

run_mode="airsim_node"

if [[ "$1" = "airsim_node" ]]; then
    shift 1
elif [[ $1 = "run_tests" ]]; then
    run_mode=$1
    shift 1
fi

echo $run_mode

bash -c "sleep 2; source /opt/ros/humble/setup.bash; ros2 topic pub \$(ros2 topic list | head -n 1) std_msgs/String '{data: \"scene_drone_sensors.jsonc\"}' --once;" &

cd /home/airsim_user/ProjectAirSim/package
bash ./start_unreal_environment.sh $@
sleep 10

echo "checking run_mode: $run_mode"

if [[ $run_mode = "airsim_node" ]]; then
    echo "Running airsim_node"
    (ros2 launch nav2_map_server map_saver_server.launch.py) &
    (sleep 10 && python3 /home/airsim_user/ProjectAirSim/ros/node/scripts/ros2/projectairsim_bridge_ros2.py --simconfigpath /home/airsim_user/ProjectAirSim/client/python/example_user_scripts/sim_config/; &)
elif [[ $run_mode = "run_tests" ]]; then
    echo "Running colcon test"
    cd ~/AirSimExtensions/ros2/
    colcon test --python-testing pytest --packages-select airsim_ros_pkgs --pytest-with-coverage
    colcon test-result --all --verbose

    #
    # colcon test-result will create a directory with the test results
    #
    cp -r build/airsim_ros_pkgs/test_results/airsim_ros_pkgs/. ~/test_output
    cp -r build/airsim_ros_pkgs/pytest_cov/. ~/test_output
fi
