"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Runs a Project AirSim Runtime flight and displays it in the drone position viewer.

Run from the repository root after starting Project AirSim Runtime. See
samples/projectairsim_runtime/README.md for Windows and Linux build/run instructions.
"""

import asyncio
import threading
from pathlib import Path

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.drone_position_viewer import DronePositionViewer
from projectairsim.utils import projectairsim_log

# Resolve the shared sim_config directory even when running from subfolders.
SIM_CONFIG_PATH = str((Path(__file__).resolve().parent.parent / "sim_config").resolve())


async def drone_sequence(drone):
    api_control_enabled = False
    armed = False
    try:
        # Set the drone to be ready to fly.
        drone.enable_api_control()
        api_control_enabled = True
        drone.arm()
        armed = True

        # Command the vehicle to take off and wait until completion.
        projectairsim_log().info("takeoff_async: starting")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task
        projectairsim_log().info("takeoff_async: completed")

        # Move up at one meter per second in NED coordinates.
        move_up_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=2.0
        )
        projectairsim_log().info("Move-Up invoked")

        await move_up_task
        projectairsim_log().info("Move-Up completed")

        # Move diagonally while maintaining altitude.
        move_diagonal_task = await drone.move_by_velocity_async(
            v_north=-1.0, v_east=1.0, v_down=0.0, duration=3.0
        )
        projectairsim_log().info("Move diagonally invoked")

        await move_diagonal_task
        projectairsim_log().info("Move diagonally completed")

        # Descend and poll task completion to keep this flow explicit.
        move_down_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=1.0, duration=2.0
        )
        projectairsim_log().info("Move-Down invoked")

        while not move_down_task.done():
            await asyncio.sleep(0.005)
        projectairsim_log().info("Move-Down completed")
    finally:
        if armed:
            drone.disarm()
        if api_control_enabled:
            drone.disable_api_control()

    projectairsim_log().info("Project AirSim Runtime demo completed")


def start_drone_thread(drone, completed_event, errors):
    def run_sequence():
        try:
            asyncio.run(drone_sequence(drone))
        except Exception as err:
            errors.append(err)
        finally:
            completed_event.set()

    thread = threading.Thread(
        target=run_sequence,
        daemon=True,
    )
    thread.start()
    return thread


async def main():
    # Create a simulation client.
    client = ProjectAirSimClient()
    connected = False
    drone_thread = None
    demo_completed = threading.Event()
    drone_errors = []

    try:
        # Connect to simulation environment.
        client.connect()
        connected = True

        # Create a World object to interact with the sim world and load a scene.
        world = World(
            client,
            "scene_basic_drone.jsonc",
            delay_after_load_sec=2,
            sim_config_path=SIM_CONFIG_PATH,
        )

        # Create a Drone object to interact with a drone in the loaded sim world.
        drone = Drone(client, world, "Drone1")
        drone_thread = start_drone_thread(drone, demo_completed, drone_errors)
        # Tk must run on the main thread. The viewer closes when the flight ends.
        DronePositionViewer(drone, close_event=demo_completed)

        drone_thread.join()
        if drone_errors:
            raise drone_errors[0]

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}")
        raise

    finally:
        # Always disconnect to allow clean reconnection in the next run.
        if connected:
            client.disconnect()
        if drone_thread is not None and drone_thread.is_alive():
            drone_thread.join()


if __name__ == "__main__":
    asyncio.run(main())
