#!/usr/bin/env python3
"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

ROS bridge for Project AirSim.  This example illustrates the basic setup of
the Project AirSim ROS Bridge node for ROS2.  Here no client script is run at
all so all interaction with the simulation must be done through the ROS
topics.  For an example that runs a mission client script in addition to the
bridge node, see "hello_ros2.py".

From a Project AirSim virtual Python environment, run this script and specify:
1. The IP address of the Project AirSim server with the "--ipaddress" flag if
   the server is not running on the local machine, and
2. The path to the simulation configuration files with the "--simconfigpath" flag
   (e.g., "../../../client/python/example_user_scripts/sim_config".)
"""

import sys
import argparse
import logging

import rclpy
import rclpy.node

from projectairsim_ros2 import ROS2Node
from projectairsim_rosbridge import ProjectAirSimROSBridge
from projectairsim.utils import projectairsim_log


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Robotic Operating System (ROS) node that bridges Project AirSim and ROS2."
    )
    parser.add_argument(
        "--address",
        help=("the IP address of the host running Project AirSim"),
        type=str,
        default="127.0.0.1",
    )
    parser.add_argument(
        "--nodename",
        help=(
            'the name for this ROS node; if not given, the node is given an anonymous name starting with "projectairsim"'
        ),
        type=str,
        default="",
    )
    parser.add_argument(
        "--simconfigpath",
        help=("the directory containing Project AirSim config files"),
        type=str,
        default="sim_config/",
    )
    parser.add_argument(
        "--topicsport",
        help=(
            "the TCP/IP port of Project AirSim's topic pub-sub client connection "
            '(see the Project AirSim command line switch "-topicsport")'
        ),
        type=int,
        default=8989,
    )
    parser.add_argument(
        "--servicesport",
        help=(
            "the TCP/IP port of Project AirSim's services client connection "
            '(see the Project AirSim command line switch "-servicessport")'
        ),
        type=int,
        default=8990,
    )
    parser.add_argument(
        "--logshowdebug", help=("display debugging log messages"), action="store_true"
    )
    args = parser.parse_args(rclpy.utilities.remove_ros_args(sys.argv)[1:])

    # Initialize ROS and start our node
    rclpy.init()
    if args.nodename == "":
        node_name = "projectairsim"
        anonymous_node_name = True
    else:
        node_name = args.nodename
        anonymous_node_name = False

    # Display debug-level messages if requested
    if args.logshowdebug:
        projectairsim_log().setLevel(logging.DEBUG)  # Display debugging messages

    # Create ROS wrapper and start ROS operations
    projectairsim_log().info(
        f'Starting ROS node "{node_name}"{" anonymized" if anonymous_node_name else ""}'
    )
    ros_node = ROS2Node(name=node_name, anonymous=anonymous_node_name)
    if anonymous_node_name:
        projectairsim_log().info(f'ROS node name is "{ros_node.name}"')
    projectairsim_ros_bridge = ProjectAirSimROSBridge(
        ros_node=ros_node,
        address=args.address,
        port_topics=args.topicsport,
        port_services=args.servicesport,
        sim_config_path=args.simconfigpath,
    )

    # Process ROS messages until stopped
    projectairsim_log().info("Project AirSim ROS2 Bridge ready")
    try:
        ros_node.spin()
    except KeyboardInterrupt:
        pass

    # Shut down ROS (and our ROS node)
    rclpy.shutdown()