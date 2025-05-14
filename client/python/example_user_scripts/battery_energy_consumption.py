"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates how to use/interact with battery sensor configured for energy consumption
mode.
"""

import asyncio
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay

client = None
world = None
drone = None


def connect_to_sim_server():
    global client, world, drone

    # Create a Project AirSim client and connect
    server = "127.0.0.1"
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a World object to interact with the sim world
    world = World(client, "scene_battery_energy.jsonc", delay_after_load_sec=0)

    # Create a Drone object to interact with a drone in the sim world
    drone = Drone(client, world, "Drone1")


# Async main function to wrap async drone commands
async def main():
    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment
        connect_to_sim_server()

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to battery sensor
        states = []
        client.subscribe(
            drone.sensors["Battery"]["battery"],
            lambda _, state: states.append(state),
        )

        image_display.start()

        # Wait a bit for image data to start displaying at client
        await asyncio.sleep(1)

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # ------------------------------------------------------------------------------

        projectairsim_log().info(
            f"Getting Battery drain. This is dynamic and depends on the drone activity"
        )
        bat_drain_rate = drone.get_battery_drain_rate("Battery")
        projectairsim_log().info(f"Battery Drain rate before API call {bat_drain_rate}")

        # ------------------------------------------------------------------------------

        # Takeoff
        projectairsim_log().info("**** Takeoff started ****")
        meters_up = 5
        vel = 2.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        move_task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=cur_pos["z"] - meters_up,
            velocity=vel,
        )

        projectairsim_log().info(f"Getting Battery drain during takeoff")
        await asyncio.sleep(1.0)
        bat_drain_rate = drone.get_battery_drain_rate("Battery")
        projectairsim_log().info(
            f"Instantaneous Battery Drain at a random moment during takeoff {bat_drain_rate} watts"
        )

        await move_task

        # ------------------------------------------------------------------------------

        projectairsim_log().info(f"Printing battery remaining every 100 entries")
        for i in range(0, int(len(states) / 100)):
            projectairsim_log().info(f"Battery status entry {i*100}")
            projectairsim_log().info(
                f"Charge level in percentage: {states[i*100]['battery_pct_remaining']}, Estimated time left in seconds: {states[i*100]['estimated_time_remaining']}"
            )

        # ------------------------------------------------------------------------------

        # Command the Drone to move down / land
        projectairsim_log().info("land_async: starting")
        land_task = await drone.land_async()

        projectairsim_log().info(f"Getting Battery drain during landing")
        await asyncio.sleep(1.0)
        bat_drain_rate = drone.get_battery_drain_rate("Battery")
        projectairsim_log().info(
            f"Instantaneous Battery Drain during at a random moment during landing {bat_drain_rate} watts"
        )

        await land_task

        # ------------------------------------------------------------------------------

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
