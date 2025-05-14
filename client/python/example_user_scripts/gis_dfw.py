"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates flying a single FastPhysics drone in a Dallas/Fort Worth (DFW) GIS scene.
A sim-packaged landing pad is spawned automatically through the scene config file.

Note: The local file path for the DFW GIS glTF tiles needs to be set in
      "tiles-dir" in sim_config/scene_gis_dfw.jsonc
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
    world = World(client, "scene_gis_dfw.jsonc", delay_after_load_sec=0)
    projectairsim_log().info("**** Loaded Dallas/Fort Worth scene ****")

    # Create a Drone object to interact with a drone in the sim world
    drone = Drone(client, world, "Drone1")


# Async main function to wrap async drone commands
async def main():
    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    try:
        # Connect to simulation environment
        connect_to_sim_server()

        # ------------------------------------------------------------------------------

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to down-facing camera sensor
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

        # Wait a bit for image data to start displaying at client
        await asyncio.sleep(3)

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Takeoff
        projectairsim_log().info("**** Takeoff started ****")
        meters_up = 30.0
        vel = 2.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=cur_pos["z"] - meters_up,
            velocity=vel,
        )
        await task

        # Command the Drone to move down / land
        projectairsim_log().info("**** Landing started ****")
        vel = 2.0
        dur = 15.0
        task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=vel, duration=dur
        )
        await task

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

        await asyncio.sleep(3)

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
