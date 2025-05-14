"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates how to write validation module that integrates
with the ValidationBench object in Airsim client. 

This can be injected into any other Airsim client script. 
This script validates that a certain 'destination' is reached and
an object of interest has been flown by during the missing. 
"""

from projectairsim.validate import ValidationTaskModule, Strictness, ReachTargetType
from projectairsim import Drone, World
from projectairsim.utils import projectairsim_log

_validation_task_module = None
# Method to call inside other scripts to summarize all injected tasks.
def summarize_validation_tasks(filename="validation-output.xml"):
    _validation_task_module.summarize(filename)

# Method to call inside other scripts to inject tasks.
def inject_validation_tasks(validation_task_module: ValidationTaskModule):
    global _validation_task_module
    _validation_task_module = validation_task_module
    add_mission_constraints()


# Parameters of the mission
def add_mission_constraints():
    object_of_interest = "Turbine"
    _validation_task_module.add_reach_validation_task(
        "Never be closer than 10m of Wind Turbine object",
        destination_type=ReachTargetType.STATIC_SCENE_OBJECT,
        destination=object_of_interest,
        error_thresh=10,
        strictness=Strictness.NEVER,
    )
    _validation_task_module.add_reach_validation_task(
        "Reach within 50m of Destination",
        destination_type=ReachTargetType.GPS_DICT,
        destination=_validation_task_module.drone.home_geo_point,
        error_thresh=50,
        strictness=Strictness.ATLEAST_ONCE,
    )
    projectairsim_log().info(
        f"Added Safety constraint: Never closer than 10m for Wind Turbine"
    )
    add_simple_constraints()


def add_simple_constraints():
    # gps_alt is the param based on "altitude" published as part of GPS sensor topic
    gps_alt = _validation_task_module.create_sensor_validation_param(
        sensor_name="GPS", sensor_type="gps", variable_path="altitude"
    )
    spawn_altitude = _validation_task_module.drone.home_geo_point["altitude"]

    if gps_alt:
        projectairsim_log().info(f"Added GPS Altitude validation task.")
        # Verifies that we never go above 100 meters our spawn altitude or below 100 meters of spawn altitude
        _validation_task_module.add_range_validation_task(
            "Altitude limits",
            gps_alt,
            min_val=spawn_altitude - 100,
            max_val=spawn_altitude + 100,
            strictness=Strictness.ALWAYS,
        )
