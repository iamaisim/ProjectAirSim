"""
For connecting to the AirSim drone environment and testing API functionality
"""
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
import projectairsim.rc

import os
import tempfile
import pprint
import sys

# connect to the Project AirSim simulator
client = ProjectAirSimClient()
client.connect()
world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
drone = Drone(client, world, "Drone1")
simple_flight_rc = projectairsim.rc.SimpleFlightRC(client, "Drone1")
try:
    drone.enable_api_control()
    drone.arm()
    # RC commands do not work if API control is enabled
    drone.disable_api_control()

    kinematics = drone.get_ground_truth_kinematics()
    s = pprint.pformat(kinematics)
    projectairsim_log().info("kinematics: %s" % s)

    # Create the RC configuration object
    rc_config = projectairsim.rc.RCConfig()
    rc_config_filename = "sim_config/xbox_rc_config.jsonc"
    projectairsim_log().info(f'Loading RC config file "{rc_config_filename}"')

    simple_flight_rc.rc_config = rc_config

    is_rc_loaded = False
    try:
        rc_config.load(rc_config_filename)
        is_rc_loaded = True
    except FileNotFoundError:
        projectairsim_log().error(f"Can't load RC config file: {sys.exc_info()[1]}")

    projectairsim_log().info('Manual mode is setup. Press enter to send RC data to takeoff')
    input()
    
    # xLeft=roll, xRight=yaw, yLeft=throttle, yRight=pitch
    channels = {"xLeft":0, "xRight":0, "yLeft":32767, "yRight":0, "switchLevelRate":0, "switchEnableAPIControl":0, "btnStart":0, "btnBack":0}
    simple_flight_rc.set(channels)

    # display the RC input sent to the flight controller
    projectairsim_log().info("RC input channels: ")
    with simple_flight_rc._lock:
        for channel in simple_flight_rc._channels:
            projectairsim_log().info(f"{channel:> 8.4f} ")
    projectairsim_log().info('\r')

    projectairsim_log().info('Press enter to set Yaw and pitch to 0.5')
    input()

    channels = {"xLeft":0, "xRight":16383, "yLeft":32767, "yRight":16383, "switchLevelRate":0, "switchEnableAPIControl":0, "btnStart":0, "btnBack":0}
    simple_flight_rc.set(channels)

    # display the RC input sent to the flight controller
    projectairsim_log().info("RC input channels: ")
    with simple_flight_rc._lock:
        for channel in simple_flight_rc._channels:
            projectairsim_log().info(f"{channel:> 8.4f} ")
    projectairsim_log().info('\r')

    projectairsim_log().info('Press enter to end program')
    input()
finally:
    simple_flight_rc.stop()
    client.disconnect()