"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates using set_wind_velocity and get_wind_velocity APIs. The set_external_force()
API is used to showcase an alternative for how a wind force can be achieved.
"""

import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay
from projectairsim.types import WeatherParameter

client = None
world = None
drone = None
private_assets = []


def connect_to_sim_server(config_file):
    global client, world, drone

    # Create a Project AirSim client and connect
    server = "127.0.0.1"
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a World object to interact with the sim world
    world = World(client, config_file, delay_after_load_sec=0)

    # Create a Drone object to interact with a drone in the sim world
    drone = Drone(client, world, "Drone1")


# Async main function to wrap async drone commands
async def main():
    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment with non-zero wind velocity configured
        connect_to_sim_server("scene_drone_wind.jsonc")

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        image_display.start()

        # Wait a bit for image data to start displaying at client
        await asyncio.sleep(1)

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Enable Weather Effects in the scene
        world.enable_weather_visual_effects()

        # Add light rain to help visualize wind
        projectairsim_log().info("Adding rain to visualize wind")
        world.set_weather_visual_effects_param(param=WeatherParameter.RAIN, value=0.3)

        # Should be the value set inside config file
        (vel_x, vel_y, vel_z) = world.get_wind_velocity()
        projectairsim_log().info(
            f"Initial wind velocity: x={vel_x}, y={vel_y}, z={vel_z}"
        )

        # Takeoff with wind, you can visually see that the drone tilts proportionally
        # to wind speed & direction
        projectairsim_log().info("**** Takeoff started ****")
        meters_up = 5
        vel = 2.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=cur_pos["z"] - meters_up,
            velocity=vel,
        )

        # Wait to see the tilt visually
        await asyncio.sleep(5)

        # This should make the tilt go away
        projectairsim_log().info("Set wind velocity to x=0.0, y=0.0, z=0.0")
        world.set_wind_velocity(0.0, 0.0, 0.0)

        # ------------------------------------------------------------------------------

        # Command the drone to move down / land
        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()

        # Tilts in a different direction since the wind is blowing in a different
        # direction
        await asyncio.sleep(5)
        projectairsim_log().info(
            "Simulating wind force with external force by applying slight force in y-direction."
        )
        drone.set_external_force([0, -0.3, 0])
        await task

        await land_task

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        world.disable_weather_visual_effects()

        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
