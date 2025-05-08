import os

from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    lifecycle_nodes = ['map_saver']
    autostart = True
    save_map_timeout = 2.0
    free_thresh_default = 0.25
    occupied_thresh_default = 0.65
    params = os.path.join(get_package_share_directory('projectairsim_ros'), 'params', 'params.yaml')

    # Nodes launching commands
    start_lifecycle_manager_cmd = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager',
        output='screen',
        emulate_tty=True,  # https://github.com/ros2/launch/issues/188
        parameters=[
            {'autostart': autostart},
            {'node_names': lifecycle_nodes},
            [params]
        ],
    )

    # Nodes launching commands
    start_map_saver_server_cmd = Node(
        package='nav2_map_server',
        executable='map_saver_server',
        name='map_saver',
        output='screen',
        emulate_tty=True,  # https://github.com/ros2/launch/issues/188
        parameters=[
            {'save_map_timeout': save_map_timeout},
            {'free_thresh_default': free_thresh_default},
            {'occupied_thresh_default': occupied_thresh_default},
            [params]
        ]
    )  

    ld = LaunchDescription()
    ld.add_action(start_map_saver_server_cmd)
    ld.add_action(start_lifecycle_manager_cmd) 
    return ld