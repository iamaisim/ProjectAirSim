#!/bin/bash
bash /home/airsim_user/ProjectAirSim/packages/Blocks/Development/Linux/Blocks.sh &
sleep 2;
bash -c "sleep 2; source /opt/ros/humble/setup.bash; ros2 topic pub \$(ros2 topic list | head -n 1) std_msgs/String '{data: \"scene_drone_sensors.jsonc\"}' --once;" &
source /opt/ros/humble/setup.bash;
source /home/airsim_user/ProjectAirSim/ros/node/install/setup.bash;
python3 /home/airsim_user/ProjectAirSim/ros/node/scripts/ros2/projectairsim_bridge_ros2.py --simconfigpath /home/airsim_user/ProjectAirSim/client/python/example_user_scripts/sim_config/;
