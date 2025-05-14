"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates manually stepping a simulation in lock step with sensor data
using sim clock pause-on-start config setting and ContinueFor* client APIs.
"""

import asyncio
import numpy as np
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log


def nanos_to_sec(nanos):
    return nanos / 1e9


class DroneWithDataSync(Drone):
    image_data = None
    pose_data = None
    target_simtime = 0  # nanosec
    image_sync = False
    pose_sync = False

    def ImageCallback(self, topic, image_msg):
        self.image_data = image_msg
        if image_msg["time_stamp"] >= self.target_simtime:
            self.image_sync = True
            projectairsim_log().info(
                f"image received at simtime={image_msg['time_stamp']},"
                f" xyz={image_msg['pos_x']}, {image_msg['pos_y']}, {image_msg['pos_z']}"
            )

    def PoseCallback(self, topic, pose_msg):
        self.pose_data = pose_msg
        if pose_msg["time_stamp"] >= self.target_simtime:
            self.pose_sync = True
            projectairsim_log().info(
                f"pose  received at simtime={pose_msg['time_stamp']},"
                f" xyz={pose_msg['position']['x']},"
                f" {pose_msg['position']['y']},"
                f" {pose_msg['position']['z']}"
            )

    def set_target_sim_time(self, t_tgt):
        self.target_simtime = t_tgt  # nanosec
        self.image_sync = False
        self.pose_sync = False

    def IsDataSynced(self) -> bool:
        return self.pose_sync and self.image_sync

    def DisplayImage(self):
        if self.image_data is not None:
            nparr = np.frombuffer(self.image_data["data"], dtype="uint8")
            img_np = np.reshape(
                nparr, [self.image_data["height"], self.image_data["width"], 3]
            )
            cv2.imshow("RGB", img_np)
            cv2.waitKey(5)


# Async main function to wrap async drone commands
async def main(client: ProjectAirSimClient, world: World, drone: DroneWithDataSync):
    # ------------------------------------------------------------------------------
    # Subscribe to the Drone's sensors with a callback to receive the sensor data

    client.subscribe(drone.sensors["DownCamera"]["scene_camera"], drone.ImageCallback)
    client.subscribe(drone.robot_info["actual_pose"], drone.PoseCallback)

    # ------------------------------------------------------------------------------
    # Step the sim manually while waiting to receive the pose/image data at each step

    world.pause()  # in case not already be paused-on-start by sim clock config
    t_now = world.get_sim_time()

    dt_simclock = (
        5e6  # 5 ms set to match config scene_manual_step_drone.jsonc 'step-ns'
    )

    # ------------------------------------------------------------------------------
    # Allow the drone to fall to a landed position before starting flying commands

    drone.set_target_sim_time(1e9)
    world.continue_for_sim_time(1e9)

    # Spin until data with target timestamp is received
    while not drone.IsDataSynced():
        await asyncio.sleep(0.005)

    t_now = world.get_sim_time()

    # Set the drone to be ready to fly
    drone.enable_api_control()
    drone.arm()

    # ------------------------------------------------------------------------------
    # Example 1 - continue_for_single_step

    for i in range(4):
        projectairsim_log().info(
            "------------------------------------------------------"
        )
        projectairsim_log().info(f"continue_for_single_step #{i+1}")

        # Send drone command for this step
        await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-5.0, duration=nanos_to_sec(dt_simclock)
        )

        # Set target timestamp for data to wait for, set sim to continue for single step
        drone.set_target_sim_time(t_now + dt_simclock)
        world.continue_for_single_step()

        # Spin until data with target timestamp is received
        while not drone.IsDataSynced():
            await asyncio.sleep(0.005)

        # Display image with target timestamp to confirm data is valid
        drone.DisplayImage()

        t_now = world.get_sim_time()

    # ------------------------------------------------------------------------------
    # Example 2 - continue_for_n_steps

    n_steps = 2
    for i in range(4):
        projectairsim_log().info(
            "------------------------------------------------------"
        )
        projectairsim_log().info(f"continue_for_n_steps #{i+1}")

        # Send drone command for this step
        await drone.move_by_velocity_async(
            v_north=0.0,
            v_east=0.0,
            v_down=-5.0,
            duration=nanos_to_sec(n_steps * dt_simclock),
        )

        # Set target timestamp for data to wait for, set sim to continue for single step
        drone.set_target_sim_time(t_now + n_steps * dt_simclock)
        world.continue_for_n_steps(n_steps)

        # Spin until data with target timestamp is received
        while not drone.IsDataSynced():
            await asyncio.sleep(0.005)

        # Display image with target timestamp to confirm data is valid
        drone.DisplayImage()

        t_now = world.get_sim_time()

    # ------------------------------------------------------------------------------
    # Example 3 - continue_for_sim_time

    dt_tgt = 20 * dt_simclock  # 100 ms
    for i in range(4):
        projectairsim_log().info(
            "------------------------------------------------------"
        )
        projectairsim_log().info(f"continue_for_sim_time #{i+1}")

        # Send drone command for this step
        await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-5.0, duration=nanos_to_sec(dt_tgt)
        )

        # Set target timestamp for data to wait for, set sim to continue for target dt
        drone.set_target_sim_time(t_now + dt_tgt)
        world.continue_for_sim_time(dt_tgt, wait_until_complete=False)

        # Spin until data with target timestamp is received
        while not drone.IsDataSynced():
            await asyncio.sleep(0.005)

        # Display image with target timestamp to confirm data is valid
        drone.DisplayImage()

        t_now = world.get_sim_time()

    # ------------------------------------------------------------------------------
    # Example 4 - continue_until_sim_time

    t_tgt = 2e9  # continue until t = 2.0 sec for initial loop
    for i in range(4):
        projectairsim_log().info(
            "------------------------------------------------------"
        )
        projectairsim_log().info(f"continue_until_sim_time #{i+1}")

        # Send drone command for this step
        await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-5.0, duration=nanos_to_sec(t_tgt - t_now)
        )

        # Set target timestamp for data to wait for, set sim to continue for target dt
        drone.set_target_sim_time(t_tgt)
        world.continue_until_sim_time(t_tgt, wait_until_complete=False)

        # Spin until data with target timestamp is received
        while not drone.IsDataSynced():
            await asyncio.sleep(0.005)

        # Display image with target timestamp to confirm data is valid
        drone.DisplayImage()

        t_now = world.get_sim_time()
        t_tgt = t_tgt + 1e9  # continue for 1 additional sec each loop

    # ------------------------------------------------------------------------------
    # Shut down the drone
    drone.disarm()
    drone.disable_api_control()


if __name__ == "__main__":
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(client, "scene_manual_step_drone.jsonc", delay_after_load_sec=0)

        # Create a Drone object to interact with a drone in the loaded sim world
        drone = DroneWithDataSync(client, world, "Drone1")

        asyncio.run(main(client, world, drone))  # Runner for async main function

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()
