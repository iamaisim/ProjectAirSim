"""
Copyright (C) Microsoft Corporation. All rights reserved.
"""

import os
import time

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, TimerAction, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration
import launch_ros.actions
from launch.launch_description_sources import PythonLaunchDescriptionSource

import launch_pytest
import launch_testing
import launch_testing.actions

from ament_index_python.packages import get_package_share_directory

import pytest

from std_msgs.msg import *
from builtin_interfaces.msg import *
from geometry_msgs.msg import *
from projectairsim_ros.srv import *
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image

from collections import defaultdict

@launch_pytest.fixture(scope='module')
def launch_description():
    ld = LaunchDescription()

    ld.add_action(DeclareLaunchArgument('nodename', default_value='IntegrationTestNode'))
    ld.add_action(DeclareLaunchArgument('simconfigpath', default_value='/home/jnovak/Projects/ProjectAirSim_new_ros/client/python/example_user_scripts/sim_config/'))
    #ld.add_action(DeclareLaunchArgument('ipaddress', default_value='127.0.0.1'))
    #ld.add_action(DeclareLaunchArgument('topicsport', default_value='8989'))
    #ld.add_action(DeclareLaunchArgument('servicesport', default_value='8990'))
    #ld.add_action(DeclareLaunchArgument('logshowdebug', default_value='true'))
    ld.add_action(DeclareLaunchArgument('unrealenvscript', default_value='/home/jnovak/Projects/ProjectAirSim_new_ros/packages/Blocks/Shipping/Linux/Blocks.sh'))
    ld.add_action(DeclareLaunchArgument('unrealoptions', default_value=''))

    projectairsim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('projectairsim_ros'), 'launch/projectairsim.launch.py')
        )
    )

    ready_to_test = TimerAction(
        period=20.0,
        actions=[
            launch_testing.actions.ReadyToTest()
        ]
    )

    ld.add_action(projectairsim_launch)
    ld.add_action(launch_testing.util.KeepAliveProc())
    ld.add_action(ready_to_test)
    rclpy.init(args=None)
    yield ld

    #
    #  Kill running blocks process and ros2 core
    #
    try:
        time.sleep(15)
        os.system("pgrep Blocks|xargs kill -9")
    except:
        pass

    time.sleep(5)

def test_case_noerrormesage():
    assert True

