
import os
import sys

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, TimerAction, IncludeLaunchDescription, OpaqueFunction, DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory

def anonymous_name(id):
    """
    Generate a ROS-legal 'anonymous' name

    @param id: prefix for anonymous name
    @type  id: str
    """
    import random
    import socket
    name = '%s_%s_%s_%s' % (id, socket.gethostname(), os.getpid(), random.randint(0, sys.maxsize))
    # RFC 952 allows hyphens, IP addrs can have '.'s, both
    # of which are illegal for ROS names. For good
    # measure, screen ipv6 ':'.
    name = name.replace('.', '_')
    name = name.replace('-', '_')
    return name.replace(':', '_')

def evaluate_airsimnode(context, *args, **kwargs):
    nodename = LaunchConfiguration('nodename').perform(context)
    simconfigpath = LaunchConfiguration('simconfigpath').perform(context)
    ipaddress = LaunchConfiguration('ipaddress').perform(context)
    topicsport = LaunchConfiguration('topicsport').perform(context)
    servicesport = LaunchConfiguration('servicesport').perform(context)
    logshowdebug = LaunchConfiguration('logshowdebug').perform(context)
    scenefile = LaunchConfiguration('scenefile').perform(context)
    namespace = LaunchConfiguration('namespace').perform(context)
    params = os.path.join(get_package_share_directory('projectairsim_ros'), 'params', 'params.yaml')

    if nodename == "Anonymous":
        nodename = anonymous_name("projectairsim")

    listArgs = [
        '--simconfigpath=' + simconfigpath,
        '--nodename='+ nodename,
        '--address=' + ipaddress,
        '--topicsport=' + topicsport,
        '--servicesport=' + servicesport,
    ]

    if logshowdebug == 'true':
        listArgs.append('--logshowdebug')

    AirSimNode = TimerAction(
        period=10.0,
        actions=[
            Node(
                package='projectairsim_ros', 
                executable='projectairsim_bridge_ros2.py',
                namespace=namespace,
                arguments= listArgs,
                parameters =[
                    {'scenefile': LaunchConfiguration('scenefile')},
                    [params]
                ],
            )
        ]
    )        

    listArgsScene = ['--scenefile=' + scenefile]
    execPub =  TimerAction(
        period=10.0,
        actions=[
            Node(
                package='projectairsim_ros', 
                executable='load_scene_node.py',
                namespace=namespace,
                arguments= listArgsScene,
                parameters =[
                    {'scenefile': LaunchConfiguration('scenefile')},
                    [params]
                ],
            )
        ],
    )

    return [ execPub, AirSimNode ]

def generate_launch_description():
    unrealenvscript = LaunchConfiguration('unrealenvscript')
    unrealoptions   = LaunchConfiguration('unrealoptions')
    enablepx4components = LaunchConfiguration('enablepx4')
    
    simconfigpath = '/home/airsim_user/ProjectAirSim/client/python/example_user_scripts/sim_config/'

    execUnrealEnv = TimerAction(period=2.0,
        actions=[
            ExecuteProcess(cmd=['bash', unrealenvscript, unrealoptions], respawn=True, respawn_delay=2.0)
        ]
    )

    map_saver_with_lifecycle = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('projectairsim_ros'), 'launch/map_saver_with_lifecycle.launch.py')
        )
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'nodename',
            default_value="Anonymous",
            description='Node name for ProjectAirSim ROS2 node'),   
        DeclareLaunchArgument(
            'simconfigpath',
            default_value='/home/airsim_user/ProjectAirSim/client/python/example_user_scripts/sim_config/',
            description='the directory containing Project AirSim config files'),   
        DeclareLaunchArgument(
            'ipaddress',
            default_value='127.0.0.1',
            description='the IP address of the host running Project AirSim'),   
        DeclareLaunchArgument(
            'topicsport',
            default_value='8989',
            description='the TCP/IP port of Project AirSim''s topic pub-sub client connection'),   
        DeclareLaunchArgument(
            'servicesport',
            default_value='8990',
            description='the TCP/IP port of Project AirSim''s services client connection'),   
        DeclareLaunchArgument(
            'logshowdebug',
            default_value='false',
            description='Debug logging'),
        DeclareLaunchArgument(
            'enablepx4',
            default_value='false',
            description='Enable PX4 components'),
        DeclareLaunchArgument(
            'unrealenvscript',
            default_value='/home/airsim_user/ProjectAirSim/packages/start_unreal_environment.sh',
            description='Path to Unreal Environment script'),   
        DeclareLaunchArgument(
            'unrealoptions',
            default_value='',
            description='Unreal Environment script options (ie -RenderOffscreen)'),   
        DeclareLaunchArgument(
            'scenefile',
            default_value='scene_drone_sensors.jsonc',
            description='ProjectAirSim scene configuration file'),   
        DeclareLaunchArgument(
            'namespace',
            default_value='',
            description='ProjectAirSim namespace for this launch file'),   
        execUnrealEnv,
        map_saver_with_lifecycle,
        OpaqueFunction(function=evaluate_airsimnode),
    ])
