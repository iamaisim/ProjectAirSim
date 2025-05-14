"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates how to use Project Airsim TestSuite to test various missions.
This uses a  validation module, fault injection module and tests the
hello drone mission through a set of domain randomizations. 
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.test import TestBench, SceneRandomizationSpec, EnvProfile
from projectairsim.utils import projectairsim_log
from projectairsim.types import WeatherParameter
from projectairsim.image_utils import ImageDisplay
import validation_module_example, fault_injection_module
from datetime import datetime


async def test_using_test_suite():
    # Create a Project AirSim client
    client = ProjectAirSimClient()
    # Initialize an ImageDisplay object to display camera sub-windows
    try:
        # List of Time of Day of interest (two random times)
        tod_of_interest = [datetime.now(), datetime(2022, 3, 4, 17, 0, 0)]
        # Lists Weather conditions of interest
        weather_of_interest = [
            [WeatherParameter.SNOW, 0.2],
            [WeatherParameter.RAIN, 0.3],
        ]

        # Initalize SceneGenerationSpec with scenes of interest, currently only one scene  can add ,"scene_two_drones.jsonc"
        scene_randomization_spec = SceneRandomizationSpec(
            list_of_scenes=["scene_basic_drone.jsonc"]
        )
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
        # Add extra test scenarios to the test suite if you desire
        custom_test_cases = []
        # Test case is a combination of scene and environment profile
        rainy_day = [
            "scene_basic_drone.jsonc",
            EnvProfile(
                weather=[WeatherParameter.RAIN, 0.3],
                time_of_day=datetime(2022, 3, 4, 8, 0, 0),
                cloud_shadow_strength=1.0,
                wind_velocity=[2, 2, 2],
            ),
        ]
        custom_test_cases.append(rainy_day)
        test_bench.add_test_scenarios(custom_test_cases)

        test_bench.save_test_bench("test.csv")

        new_bench = TestBench.load_test_bench_from_file(
            "test.csv", f"testbench-{datetime.now().strftime('%Y-%m-%d')}"
        )

        # Connect to simulation environment
        client.connect()
        # Run the test bench
        await new_bench.test_and_validate_scenarios(client, mission_script, "Drone1")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


async def mission_script(drone: Drone, world: World, client: ProjectAirSimClient):
    image_display = ImageDisplay()
    # Subscribe to chase camera sensor as a client-side pop-up window
    chase_cam_window = "ChaseCam"
    image_display.add_chase_cam(chase_cam_window)
    client.subscribe(
        drone.sensors["Chase"]["scene_camera"],
        lambda _, chase: image_display.receive(chase, chase_cam_window),
    )

    # Subscribe to the downward-facing camera sensor's RGB and Depth images
    rgb_name = "RGB-Image"
    image_display.add_image(rgb_name, subwin_idx=0)
    client.subscribe(
        drone.sensors["DownCamera"]["scene_camera"],
        lambda _, rgb: image_display.receive(rgb, rgb_name),
    )

    depth_name = "Depth-Image"
    image_display.add_image(depth_name, subwin_idx=2)
    client.subscribe(
        drone.sensors["DownCamera"]["depth_camera"],
        lambda _, depth: image_display.receive(depth, depth_name),
    )

    image_display.start()

    # ------------------------------------------------------------------------------

    # Set the drone to be ready to fly
    drone.enable_api_control()
    drone.arm()

    # ------------------------------------------------------------------------------

    projectairsim_log().info("takeoff_async: starting")
    takeoff_task = (
        await drone.takeoff_async()
    )  # schedule an async task to start the command

    # Example 1: Wait on the result of async operation using 'await' keyword
    await takeoff_task
    projectairsim_log().info("takeoff_async: completed")

    # ------------------------------------------------------------------------------

    # Command the drone to move up in NED coordinate system at 1 m/s for 4 seconds
    move_up_task = await drone.move_by_velocity_async(
        v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
    )
    projectairsim_log().info("Move-Up invoked")

    await move_up_task
    projectairsim_log().info("Move-Up completed")

    # ------------------------------------------------------------------------------

    # Command the Drone to move down in NED coordinate system at 1 m/s for 4 seconds
    move_down_task = await drone.move_by_velocity_async(
        v_north=0.0, v_east=0.0, v_down=1.0, duration=4.0
    )  # schedule an async task to start the command
    projectairsim_log().info("Move-Down invoked")

    # Example 2: Wait for move_down_task to complete before continuing
    while not move_down_task.done():
        await asyncio.sleep(0.005)
    projectairsim_log().info("Move-Down completed")

    # ------------------------------------------------------------------------------

    projectairsim_log().info("land_async: starting")
    land_task = await drone.land_async()
    await land_task
    projectairsim_log().info("land_async: completed")

    # ------------------------------------------------------------------------------

    # Shut down the drone
    drone.disarm()
    drone.disable_api_control()

    image_display.stop()
    return


if __name__ == "__main__":
    asyncio.run(test_using_test_suite())
