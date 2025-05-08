#!/usr/bin/env python3
"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demo client script for building and saving a 3D occupancy map
using the Project AirSim ROS bridge, the MoveIt! ROS motion
package, and a mission script.

Do not run this script directly!  Instead, from a Python
virtual environment setup for the Project AirSim client and
ROS packages, run the command:

    roslaunch projectairsim_ros_examples build_map.launch
"""
import argparse
import asyncio
import pathlib
import traceback
import logging
import sys

import rospy

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

        projectairsim_log().info("Move up")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-4.0, duration=4.0
        )
        await move_task

        start_pos = drone.get_ground_truth_kinematics()["pose"]["position"]

        # Fly the drone around the scene
        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=12.0
        )
        await move_task

        projectairsim_log().info("Move north-east")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=4.0, v_down=0.0, duration=8.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=3.0
        )
        await move_task

        projectairsim_log().info("Move down")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=4.0, duration=4.0
        )
        await move_task

        await asyncio.sleep(2)

        projectairsim_log().info("Move up")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-4.0, duration=2.0
        )
        await move_task

        projectairsim_log().info("Move south")
        move_task = await drone.move_by_velocity_async(
            v_north=-4.0, v_east=0.0, v_down=0.0, duration=3.0
        )
        await move_task

        projectairsim_log().info("Move south-west")
        move_task = await drone.move_by_velocity_async(
            v_north=-4.0, v_east=-4.0, v_down=0.0, duration=8.0
        )
        await move_task

        # Fly back to start position
        projectairsim_log().info("Move back to start")
        move_task = await drone.move_to_position_async(
            north=start_pos["x"], east=start_pos["y"], down=start_pos["z"], velocity=4
        )
        await move_task

        projectairsim_log().info("Move down")
        move_task = await drone.move_to_position_async(
            north=start_pos["x"], east=start_pos["y"], down=-3.0, velocity=4
        )
        await move_task

        projectairsim_log().info("Land")
        land_task = await drone.land_async()
        await land_task

        projectairsim_log().info("Scan mission complete")
        drone.disarm()
        drone.disable_api_control()

    except asyncio.CancelledError:
        pass

    except:
        print(f"do_mission() caught exception: {traceback.format_exc()}")
        raise


async def main(client, world):
    try:
        # Launch mission task
        mission_task = asyncio.create_task(do_mission(client, world))

        # Wait for mission task to complete or ROS to be shutdown
        while True:
            tasks_done, _ = await asyncio.wait({mission_task}, timeout=1)
            if mission_task in tasks_done:
                break

            if rospy.is_shutdown():
                break

        # Shutdown mission task if it's still running
        try:
            while not mission_task.done():
                mission_task.cancel()
                await asyncio.wait({mission_task}, timeout=1)
        except asyncio.CancelledError:
            pass

        # Save occupancy map to file
        if not rospy.is_shutdown():
            projectairsim_log().info("Processing octomap")
            octomaphandler = OctomapHandler()
            await octomaphandler.save_octomap_async("build_map_example.octomap")
            del octomaphandler
            projectairsim_log().info("Octomap processing complete")

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
    asyncio.run(main(client, world))

    # Enter ROS main loop
    rospy.spin()

    # Disconnect the client from Project AirSim
    client.disconnect()
