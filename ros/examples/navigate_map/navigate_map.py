#!/usr/bin/env python3
"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demo client script for navigating a 3D occupancy map
loaded from a file using the Project AirSim ROS bridge, and
the MoveIt! ROS motion package.  Create the 3D occupancy
map first using the "build_map" example.

Do not run this script directly!  Instead, from a Python
virtual environment setup for the Project AirSim client and
ROS packages, run the command:

    roslaunch projectairsim_ros_examples navigate_map.launch
"""
import argparse
import asyncio
import traceback
import logging
import os
import pathlib
import sys

import rospy
from sensor_msgs.msg import PointCloud2

import moveit_commander
import moveit_msgs.msg as rosmoveitmsgs

import projectairsim
from projectairsim.utils import projectairsim_log

from projectairsim_ros import ProjectAirSimWrapper

sys.path.append(
    str(pathlib.Path(__file__).parent.parent)
)  # Add examples directory to path
from common.octomap_handler import OctomapHandler


client = None  # Project AirSim client


async def do_mission(client, world):
    try:
        # Create a drone object to interact with a Drone in the loaded ProjectAirSim world
        drone = projectairsim.Drone(client, world, "Drone1")

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------

        projectairsim_log().info("Taking off")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task

    except asyncio.CancelledError:
        pass

    except:
        print(f"do_mission() caught exception: {traceback.format_exc()}")
        raise


async def main(
    client: projectairsim.ProjectAirSimClient,
    world: projectairsim.World,
    filepath_occupancy: str,
):
    try:
        # Launch mission task
        mission_task = asyncio.create_task(do_mission(client, world))

        # Load pre-existing octomap
        filepath_occupancy = os.path.abspath(filepath_occupancy)
        projectairsim_log().info(f'Loading occupancy map "{filepath_occupancy}"')
        octomaphandler = OctomapHandler()
        await octomaphandler.load_octomap(filepath_occupancy)
        del octomaphandler
        projectairsim_log().info(f'Loaded cccupancy map "{filepath_occupancy}"')

        # Wait for ROS to be shutdown
        while True:
            tasks_done, _ = await asyncio.wait({mission_task}, timeout=1)

            if rospy.is_shutdown():
                break

        # Shutdown mission task if it's still running
        try:
            while not mission_task.done():
                mission_task.cancel()
                await asyncio.wait({mission_task}, timeout=1)
        except asyncio.CancelledError:
            pass

        # Shutdown ROS if it's still running
        if not rospy.is_shutdown():
            rospy.signal_shutdown("Flight mission complete")

    except:
        print(f"main() caught exception: {traceback.format_exc()}")
        raise


def _redirecting_log_filter(record):
    """
    This log filter redirects a log record from the parent logger to our
    logger and tells the parent logger to ignore it.

    This useful because projectairsim_log() is hard-coded to use the
    "projectairsim" logger and rospy redirects all standard Python logging
    that's not directed to the "rosout" logger or a child of "rosout" to a
    log file by default.  By redirecting Project AirSim's logger to
    "rosout.projectairsim", the logs are also output to stdout by default.
    """
    logging.getLogger("rosout.projectairsim").handle(record)
    return False


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Project AirSim Robotic Operating System (ROS) node that bridges Project AirSim and ROS."
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
        default="../../../client/python/example_user_scripts/sim_config/",
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

    # Parse command-line
    args = parser.parse_args(args=rospy.myargv(argv=sys.argv)[1:])

    # Initialize ROS and start our node
    if args.nodename == "":
        node_name = "projectairsim"
        rospy.init_node(node_name, anonymous=True)
    else:
        node_name = args.nodename
        rospy.init_node(node_name, anonymous=False)

    # Redirect Project AirSim library's log output to "rosout.projectairsim" logger
    # so output is also sent to the console by default
    projectairsim_log().addFilter(_redirecting_log_filter)
    # projectairsim_log().setLevel(logging.DEBUG) # Display debugging messages

    # Create client to Project AirSim
    client = projectairsim.ProjectAirSimClient(
        address=args.address,
        port_topics=args.topicsport,
        port_services=args.servicesport,
    )
    client.connect()

    # Load the simulation scene
    world = projectairsim.World(
        client,
        "scene_drone_sensors.jsonc",
        sim_config_path=args.simconfigpath,
        delay_after_load_sec=2,
    )

    # Create ROS wrapper and start ROS operations
    projectairsim_log().info(f'Starting ROS node "{node_name}"')
    projectairsim_ros_wrapper = ProjectAirSimWrapper(
        client=client, sim_config_path=args.simconfigpath
    )

    # Start drone movement
    asyncio.run(main(client, world, "build_map_example.octomap"))

    # Enter ROS main loop
    rospy.spin()

    # Disconnect the client from Project AirSim
    client.disconnect()
