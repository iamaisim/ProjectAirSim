"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Demonstrates using a radar sensor mounted on a tower as a static sensor actor.
"""

import asyncio

from projectairsim import ProjectAirSimClient, Drone, World, StaticSensorActor
from projectairsim.utils import projectairsim_log
from projectairsim.image_utils import ImageDisplay
from projectairsim.radar_utils import RadarDisplay


def print_tracks(track_msg):
    print(f"Tracks at t = {(track_msg['time_stamp'] * 1e-9):.3f} sec:")
    for track in track_msg["radar_tracks"]:
        print(track)


# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    # Initialize an ImageDisplay object to set up sub-window positions
    image_display = ImageDisplay(num_subwin=4)
    subwin_left = image_display.get_subwin_info(0)  # use 2x wide slot 0+1
    subwin_right = image_display.get_subwin_info(2)  # use 2x wide slot 2+3

    # ----------------------------------------------------------------------------------

    # Initialize a RadarDisplay object to plot Radar Detections and Tracks
    radar_display = RadarDisplay(
        det_plot_x=subwin_left["x"],
        det_plot_y=subwin_left["y"],
        track_plot_x=subwin_right["x"],
        track_plot_y=subwin_right["y"],
    )

    # ----------------------------------------------------------------------------------

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_radar_tower.jsonc", delay_after_load_sec=2)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = Drone(client, world, "Drone1")

        # Subscribe to chase camera sensor
        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        image_display.start()

        # ------------------------------------------------------------------------------

        # Create a radar tower static sensor actor to interact with it in the loaded
        # sim world

        radar_tower = StaticSensorActor(client, world, "RadarTower1")

        client.subscribe(
            radar_tower.sensors["radar1"]["radar_detections"],
            lambda _, detection_msg: radar_display.receive_detections(
                detection_msg["radar_detections"]
            ),
        )

        client.subscribe(
            radar_tower.sensors["radar1"]["radar_tracks"],
            lambda _, tracks_msg: radar_display.receive_tracks(
                tracks_msg["radar_tracks"]
            ),
        )

        radar_display.start()

        # ------------------------------------------------------------------------------

        # Set the drone to be ready to fly
        drone.enable_api_control()
        drone.arm()

        # Fly the drone around the scene
        projectairsim_log().info("Move up")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-2.0, duration=2.0
        )
        await move_task

        projectairsim_log().info("Move forward")
        move_task = await drone.move_by_velocity_async(
            v_north=2.0, v_east=0.0, v_down=0.0, duration=6.5
        )
        await move_task

        projectairsim_log().info("Move down")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=2.0, duration=3.0
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

        radar_display.stop()

        # ------------------------------------------------------------------------------


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
