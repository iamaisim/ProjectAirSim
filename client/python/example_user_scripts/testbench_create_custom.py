"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
This script is used to generate a test bench for the Project AirSim.
Test bench is a collection of test scenarios that can be used to test the drone.
This script demonstrates how to generate a test bench with custom test scenarios.
"""

from datetime import datetime

from projectairsim.types import WeatherParameter
from projectairsim.test import TestBench, SceneRandomizationSpec, EnvProfile
from projectairsim.utils import projectairsim_log

import validation_module_example, fault_injection_module


def create_test_bench():
    # Create a Project AirSim client
    # Initialize an ImageDisplay object to display camera sub-windows
    scenes_to_use = ["scene_basic_drone.jsonc"]
    projectairsim_log().info(
        "Generating test bench with the following scenes: {}".format(scenes_to_use)
    )
    tod_of_interest = [datetime.now(), datetime(2022, 3, 4, 10, 0, 0)]
    # Lists Weather conditions of interest
    weather_of_interest = [
        [WeatherParameter.SNOW, 0.2],
        [WeatherParameter.RAIN, 0.3],
    ]

    # Initalize SceneGenerationSpec with scenes of interest, currently only one scene  can add ,"scene_two_drones.jsonc"
    scene_randomization_spec = SceneRandomizationSpec(list_of_scenes=scenes_to_use)
    # Add all the desired variations
    scene_randomization_spec.add_time_of_day_variations(tod_of_interest)
    scene_randomization_spec.add_weather_variations(weather_of_interest)
    # Generate scenario list
    test_cases = scene_randomization_spec.generate_scenario_list()

    # Initialize testsuite with validation module, fault injection module and test cases
    test_bench = TestBench(
        name="Hello Drone Test Bench",  # Name of the test bench
        validation_module=validation_module_example,
        fault_injection_module=fault_injection_module,
        scenarios=test_cases,
    )

    custom_test_cases = []
    # Test case is a combination of scene and environment profile
    rainy_day = [
        scenes_to_use[0],
        EnvProfile(
            weather=[WeatherParameter.RAIN, 0.3],
            time_of_day=datetime(2022, 3, 4, 12, 0, 0),
            cloud_shadow_strength=1.0,
            wind_velocity=[2, 2, 2],
        ),
    ]
    custom_test_cases.append(rainy_day)
    test_bench.add_test_scenarios(custom_test_cases)
    export_file = "custom_testbench.csv"

    test_bench.save_test_bench(export_file)
    projectairsim_log().info("Test bench generated and saved to {}".format(export_file))


if __name__ == "__main__":
    create_test_bench()
    # Add extra test scenarios to the test suite if you desire
