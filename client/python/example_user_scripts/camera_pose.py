"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates how to use set_camera_pose() and camera_look_at_object() APIs.
The camera frustum is rendered to illustrate the camera pose as it rotates
to maintain the object in its field of view.
"""

import asyncio
import math
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.image_utils import ImageDisplay
from projectairsim.types import Vector3, Quaternion, Pose, ImageType

client = None
world = None
drone = None
private_assets = []


def connect_to_sim_server():
    global client, world, drone

    # Create a Project AirSim client and connect
    server = "127.0.0.1"
    client = ProjectAirSimClient(server)
    client.connect()
    projectairsim_log().info("**** Connected to Sim ****")

    # Create a World object to interact with the sim world
    world = World(client, "scene_basic_drone_chase.jsonc", delay_after_load_sec=0)

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
        image_display.add_chase_cam(
            chase_name=chase_cam_window, resize_x=400, resize_y=225
        )
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        image_display.start()

        # Wait a bit for image data to start displaying at client
        await asyncio.sleep(1)

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Takeoff
        projectairsim_log().info("**** Takeoff started ****")
        meters_up = 35
        vel = 3.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=cur_pos["z"] - meters_up,
            velocity=vel,
        )
        await task

        # Move drone over to the left
        projectairsim_log().info("Move left invoked")
        move_left_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-3.0, v_down=0.0, duration=4.0
        )
        await move_left_task
        projectairsim_log().info("Move left completed.")

        # ------------------------------------------------------------------------------

        # New desired pose position and rotation for chase cam
        pos = {"x": -5.0, "y": 0.0, "z": -1.0}
        rot = {"r": 0, "p": -0.2, "y": 0}

        # Convert Eulerian angle to Quaternion
        (w, x, y, z) = rpy_to_quaternion(rot["r"], rot["p"], rot["y"])

        translation = Vector3({"x": pos["x"], "y": pos["y"], "z": pos["z"]})
        rotation = Quaternion({"w": w, "x": x, "y": y, "z": z})
        transform = {"translation": translation, "rotation": rotation}
        pose = Pose(transform)

        projectairsim_log().info("Setting chase camera to new pose.")
        drone.set_camera_pose("Chase", pose)

        # Point DownCamera at orange ball
        projectairsim_log().info("Rotating DownCamera to focus on orange ball.")
        drone.camera_look_at_object(camera_id="DownCamera", object_name="OrangeBall")

        # Enable visualization of camera frustum
        projectairsim_log().info("Enabling visualization of camera frustum.")
        drone.camera_draw_frustum("DownCamera", True, ImageType.SCENE)

        # ------------------------------------------------------------------------------

        # Wait to illustrate the change in Camera pose visually (NOT REQUIRED)
        await asyncio.sleep(1)

        # Fly CCW sequence around orange ball
        projectairsim_log().info("Executing flight sequence around orange ball.")
        move_forward_task = await drone.move_by_heading_async(
            heading=0.0, speed=5.0, duration=23
        )
        await move_forward_task

        heading_90_task = await drone.move_by_heading_async(
            heading=math.radians(90.0), speed=5.0, duration=10
        )
        await heading_90_task

        heading_180_task = await drone.move_by_heading_async(
            heading=math.radians(180.0), speed=5.0, duration=25
        )
        await heading_180_task

        heading_270_task = await drone.move_by_heading_async(
            heading=math.radians(270.0), speed=5.0, duration=10
        )
        await heading_270_task

        heading_0_task = await drone.move_by_heading_async(
            heading=math.radians(0.0), speed=3.0, duration=6
        )
        await heading_0_task

        # Reset camera poses to default
        projectairsim_log().info("Resetting cameras their original poses.")
        drone.reset_camera_pose(camera_id="Chase")
        drone.reset_camera_pose(camera_id="DownCamera")
        drone.camera_draw_frustum("DownCamera", False, ImageType.SCENE)

        # Move down
        projectairsim_log().info("**** Landing started ****")
        meters_up = 5
        vel = 3.0
        cur_pos = drone.get_ground_truth_kinematics()["pose"]["position"]
        task = await drone.move_to_position_async(
            north=cur_pos["x"],
            east=cur_pos["y"],
            down=-meters_up,
            velocity=vel,
        )
        await task

        # Command the Drone land
        land_task = await drone.land_async()
        await land_task

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
