"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Demonstrates using the GPU lidar sensor (lidar-type: gpu_cylindrical) with a
cylindrical scan pattern. This is a variant of lidar_basic.py that loads a
scene whose robot config sets "lidar-type": "gpu_cylindrical", which routes
sensor creation to UGPULidar in UnrealSensorFactory.
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay
from projectairsim.lidar_utils import LidarDisplay


# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to display camera sub-windows
    image_display = ImageDisplay()

    # ----------------------------------------------------------------------------------

    # Initialize a LidarDisplay object for a point cloud visualization sub-window.
    # Use intensity coloring — segmentation_cloud is currently filled with -1
    # by GPULidar (TODO in source), which would render every point black on
    # the black background and make the cloud invisible.
    lidar_subwin = image_display.get_subwin_info(2)
    lidar_display = LidarDisplay(
        x=lidar_subwin["x"],
        y=lidar_subwin["y"] + 30,  # add 30 y-pix for window title bar
        color_mode=LidarDisplay.COLOR_INTENSITY,
    )

    # ----------------------------------------------------------------------------------

    try:
        # Connect to simulation environment
        client.connect()

        # Load the GPU-lidar scene (robot config sets "lidar-type": "gpu_cylindrical")
        world = World(client, "scene_lidar_drone_gpu.jsonc", delay_after_load_sec=2)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Subscribe to chase camera sensor
        # chase_cam_window = "ChaseCam"
        # image_display.add_chase_cam(chase_cam_window)
        # client.subscribe(
        #     drone.sensors["Chase"]["scene_camera"],
        #     lambda _, chase: image_display.receive(chase, chase_cam_window),
        # )

        # Subscribe to the Drone's sensors with a callback to receive the sensor data
        rgb_name = "RGB-Image"
        image_display.add_image(rgb_name, subwin_idx=0)
        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: image_display.receive(rgb, rgb_name),
        )

        depth_name = "Depth-Image"
        image_display.add_image(depth_name, subwin_idx=1)
        client.subscribe(
            drone.sensors["DownCamera"]["depth_camera"],
            lambda _, depth: image_display.receive(depth, depth_name),
        )

        image_display.start()

        # ------------------------------------------------------------------------------

        lidar_msg_counter = 0

        def on_lidar(_, lidar):
            nonlocal lidar_msg_counter
            lidar_msg_counter += 1
            if lidar_msg_counter % 10 == 1:
                pc = lidar.get("point_cloud", []) if isinstance(lidar, dict) else []
                n_points = len(pc) // 3
                if n_points:
                    xs = pc[0::3]
                    ys = pc[1::3]
                    zs = pc[2::3]
                    ranges = [(x * x + y * y + z * z) ** 0.5 for x, y, z in zip(xs, ys, zs)]
                    fwd = sum(1 for x in xs if x > 0)
                    projectairsim_log().info(
                        f"[client] lidar msg #{lidar_msg_counter}: n={n_points} "
                        f"forward(x>0)={fwd} ({100*fwd//n_points}%) "
                        f"range[min={min(ranges):.2f} mean={sum(ranges)/n_points:.2f} max={max(ranges):.2f}] m, "
                        f"x[{min(xs):.2f},{max(xs):.2f}] "
                        f"y[{min(ys):.2f},{max(ys):.2f}] "
                        f"z[{min(zs):.2f},{max(zs):.2f}] "
                        f"sample={list(pc[:6])}"
                    )
                else:
                    projectairsim_log().info(
                        f"[client] lidar msg #{lidar_msg_counter}: 0 points"
                    )
            lidar_display.receive(lidar)

        client.subscribe(drone.sensors["lidar1"]["lidar"], on_lidar)

        lidar_display.start()

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Fly the drone around the scene
        projectairsim_log().info("Move up")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-3.0, duration=4.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=12.0
        )
        await move_task

        projectairsim_log().info("Move north-east")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=4.0, v_down=0.0, duration=8.0
        )
        await move_task

        projectairsim_log().info("Move north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=3.0
        )
        await move_task

        projectairsim_log().info("Move down")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=3.0, duration=4.0
        )
        await move_task

        # Shut down the drone
        drone.disarm()
        drone.disable_api_control()

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()

        image_display.stop()

        # ------------------------------------------------------------------------------

        lidar_display.stop()

        # ------------------------------------------------------------------------------


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
