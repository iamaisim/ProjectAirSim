"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates driving a rover with no camera sensors .
"""
import asyncio
from projectairsim import ProjectAirSimClient, Rover, World
from projectairsim.utils import projectairsim_log


# Stop the rover
async def brake_rover(rover, brake=0.5):
    rover_task = await rover.set_rover_controls(
        engine=0.0, steering_angle=0.0, brake=brake
    )
    await rover_task
    await rover.wait_until_stopped_async()


# Async main function to wrap async rover commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_basic_rover.jsonc", delay_after_load_sec=0)

        # Create a rover object in the loaded sim world
        rover = Rover(client, world, "Rover1")

        projectairsim_log().info(
            "Client connected with Rover and World objects created"
        )

        # ------------------------------------------------------------------------------

        # Set the Rover to be ready to drive
        projectairsim_log().info("Enabling API control")
        rover.enable_api_control()

        projectairsim_log().info("Arming vehicle")
        rover.arm()

        projectairsim_log().info("Setting rover controls to zero")
        rover_task = await rover.set_rover_controls(
            engine=0.0, steering_angle=0.0, brake=0.0
        )
        await rover_task
        await asyncio.sleep(5.0)

        projectairsim_log().info("Setting rover controls forward and to the right")
        rover_task = await rover.set_rover_controls(
            engine=0.5, steering_angle=1.0, brake=0.0
        )
        await rover_task
        await asyncio.sleep(5.0)

        projectairsim_log().info("Setting rover controls forward and to the left")
        rover_task = await rover.set_rover_controls(
            engine=0.5, steering_angle=-1.0, brake=0.0
        )
        await rover_task
        await asyncio.sleep(5.0)

        projectairsim_log().info("Braking")
        await brake_rover(rover)

        projectairsim_log().info("Setting rover controls backwards and to the right")
        rover_task = await rover.set_rover_controls(
            engine=-0.5, steering_angle=1.0, brake=0.0
        )
        await rover_task
        await asyncio.sleep(5.0)

        projectairsim_log().info("Setting rover controls backwards and to the left")
        rover_task = await rover.set_rover_controls(
            engine=-0.5, steering_angle=-1.0, brake=0.0
        )
        await rover_task
        await asyncio.sleep(5.0)

        projectairsim_log().info("Braking")
        await brake_rover(rover)

        projectairsim_log().info("Setting rover controls zero")
        rover_task = await rover.set_rover_controls(
            engine=0.0, steering_angle=0.0, brake=0.0
        )
        await rover_task
        await asyncio.sleep(1.0)

        rover.disarm()
        rover.disable_api_control()
        projectairsim_log().info("End of Rover Controls")

        # ------------------------------------------------------------------------------

    # logs exception on the console
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
