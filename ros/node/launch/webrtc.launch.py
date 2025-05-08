import os
import sys

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    namespace = LaunchConfiguration('namespace')

    return LaunchDescription([
        DeclareLaunchArgument(
            'namespace',
            default_value='',
            description='Namespace for WebRTC node'
        ),
        Node(
            package='projectairsim_ros',
            executable='webrtc_node.py',
            namespace=namespace,
            output='screen'
        )
    ])
