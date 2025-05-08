"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates how to write validation modules that can be injected
into the ValidationBench object in Airsim client. 

This can be used in any other Airsim client script. 
For examples on how to use this, see "battery_simple_with_validation.py" 
which is "battery_simple.py" but with 3 extra lines of code to inject 
this validaiton module. 
"""

from projectairsim.validate import ValidationTaskModule, Strictness, ReachTargetType
from projectairsim import Drone, World
from projectairsim.utils import projectairsim_log

_validation_task_module = None

# Method to call inside other scripts to inject tasks.
def inject_validation_tasks(validation_task_module: ValidationTaskModule):
    global _validation_task_module
    _validation_task_module = validation_task_module
    add_tasks()


# Method to call inside other scripts to summarize all injected tasks.
def summarize_validation_tasks(filename="validation_results.xml"):
    _validation_task_module.summarize(filename)


# Helper method to add various task types
def add_tasks():
    add_range_tasks()
    add_reach_tasks()


def add_range_tasks():
    add_range_task_on_robot_info_param()
    add_range_task_on_sensor_topic_param()


# Examples for Reach tasks types
def add_reach_tasks():
    projectairsim_log().info(f"Added Reach validation tasks.")

    world_home_geo_point = _validation_task_module.drone.home_geo_point
    # Never go farther than 100 meters from home
    # Use GPS_DICT target type since home_geo_point is stored as a dict
    _validation_task_module.add_reach_validation_task(
        "Never farther than 100m",
        ReachTargetType.GPS_DICT,
        world_home_geo_point,
        100,
        Strictness.ALWAYS,
    )

    # Approach within 10m of home at least once
    # Use GPS_LIST target type
    gps_list_target = [
        world_home_geo_point["latitude"],
        world_home_geo_point["longitude"],
        world_home_geo_point["altitude"],
    ]
    _validation_task_module.add_reach_validation_task(
        "Get within 10m",
        ReachTargetType.GPS_LIST,
        gps_list_target,
        10,
        Strictness.ATLEAST_ONCE,
    )

    # Never be closer than 5m to home
    _validation_task_module.add_reach_validation_task(
        "Never closer than 5m", ReachTargetType.GPS_DICT, world_home_geo_point, 5, Strictness.NEVER
    )


# Examples for tasks based on robot_info topics
def add_range_task_on_robot_info_param():
    # z_val is the param based the z value of "actual_pose" of the drone
    # This is usually published as part of {world_name}/robots/{drone_name}/actual_pose
    # However, the world_name/drone_name can be omitted since the validation_suite is scoped to the drone
    z_val = _validation_task_module.create_robot_info_validation_param(
        topic="actual_pose", variable_path="position/z"
    )
    # Notice here that the task is validating that z val is within an absolute range
    # compared to the Reach task above which is verifying that its within x meters of target
    if z_val:
        projectairsim_log().info(f"Added Pose Z value validation task.")
        # Verifies if z_val position is within range
        _validation_task_module.add_range_validation_task(
            "z_val limits",
            z_val,
            min_val=-20,
            max_val=100,
            strictness=Strictness.ALWAYS,
        )


def add_range_task_on_sensor_topic_param():
    # gps_alt is the param based on "altitude" published as part of GPS sensor topic
    gps_alt = _validation_task_module.create_sensor_validation_param(
        sensor_name="GPS", sensor_type="gps", variable_path="altitude"
    )
    spawn_altitude = _validation_task_module.drone.home_geo_point["altitude"]

    if gps_alt:
        projectairsim_log().info(f"Added GPS Altitude validation task.")
        # Verifies that we never go above 100 meters our spawn altitude or below 10 meters of spawn altitude
        _validation_task_module.add_range_validation_task(
            "Altitude limits",
            gps_alt,
            min_val=spawn_altitude - 10,
            max_val=spawn_altitude + 100,
            strictness=Strictness.ALWAYS,
        )

    # battery_pct is the param based on "battery_pct_remaining" published as part of Battery sensor topic
    battery_pct = _validation_task_module.create_sensor_validation_param(
        "Battery", "battery", "battery_pct_remaining"
    )
    if battery_pct:
        projectairsim_log().info(f"Added Battery Percentage validation task.")
        # Verifies battery percentage is never below 30%
        _validation_task_module.add_range_validation_task(
            "Battery min 30", battery_pct, 30, 100, Strictness.ALWAYS
        )
