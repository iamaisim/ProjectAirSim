"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
This script is used to generate a test bench for the Project AirSim test bench.
"""

from projectairsim.types import WeatherParameter
from datetime import datetime
from projectairsim.test import TestBench, SceneRandomizationSpec
import validation_module_example, fault_injection_module
from projectairsim.utils import projectairsim_log
import argparse

parser = argparse.ArgumentParser(description="Project AirSim Test Bench")
parser.add_argument("--output-filename", type=str, default="testbench_example.csv", help="File to export test bench to")
parser.add_argument("--scenes", type=str, nargs="+", default=["scene_basic_drone.jsonc"], help="List of scenes to use for the test bench")
parser.add_argument("--variation-spec", type=str, default="testbench_variaton_spec.jsonc", help="Variation spec to use for the test bench")

args = parser.parse_args()
export_file = args.output_filename
if (not export_file.endswith(".csv")):
    export_file += ".csv"

scenes_to_use = args.scenes
print(scenes_to_use)
def generate_test_bench():
    # Create a Project AirSim client
    # Initialize an ImageDisplay object to display camera sub-windows
    projectairsim_log().info("Generating test bench with the following scenes: {}".format(scenes_to_use))
    # Initalize SceneGenerationSpec with scenes of interest, currently only one scene  can add ,"scene_two_drones.jsonc"
    scene_randomization_spec = SceneRandomizationSpec(
        list_of_scenes=scenes_to_use
    )

    scene_randomization_spec.add_variations_from_spec(args.variation_spec)
    # Generate scenario list
    test_cases = scene_randomization_spec.generate_scenario_list()

    # Initialize testsuite with validation module, fault injection module and test cases
    test_bench = TestBench(
        name="Hello Drone Test Bench",  # Name of the test bench
        validation_module=validation_module_example,
        fault_injection_module=fault_injection_module,
        scenarios=test_cases,
    )
    

    test_bench.save_test_bench(export_file)
    projectairsim_log().info("Test bench generated and saved to {}".format(export_file))

if __name__ == "__main__":
    generate_test_bench()