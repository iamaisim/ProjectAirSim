"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Demonstrates a Simple Flight quadrotor orbiting at 20 m while periodically
moving a gimbal-mounted forward-facing camera and displaying its RGB stream.

Press Ctrl+C to land and exit.
"""

import asyncio
import math
import time

import cv2
from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.drone import YawControlMode
from projectairsim.types import ImageType
from projectairsim.utils import projectairsim_log, unpack_image


GIMBAL_ID = "FrontCameraGimbal"
CAMERA_ID = "FrontCamera"
FLIGHT_DOWN_METERS = -20.0  # NED: negative down is altitude above the origin.
ORBIT_RADIUS_METERS = 20.0
ORBIT_SPEED_METERS_PER_SEC = 4.0
ORBIT_WAYPOINT_COUNT = 24
GIMBAL_RATE_DEG_PER_SEC = 15.0
GIMBAL_HOLD_SEC = 3.0
GIMBAL_YAW_SEQUENCE_DEG = (0.0, 30.0, 60.0, 30.0, 0.0, -30.0, -60.0, -30.0)


async def fly_orbit(drone: Drone, center_north: float, center_east: float) -> None:
    """Continuously fly a clockwise circle around the takeoff position."""
    entry_north = center_north + ORBIT_RADIUS_METERS
    entry_east = center_east

    projectairsim_log().info(
        "Moving to a %.1f m orbit and flying at %.1f m/s",
        ORBIT_RADIUS_METERS,
        ORBIT_SPEED_METERS_PER_SEC,
    )
    entry_task = await drone.move_to_position_async(
        north=entry_north,
        east=entry_east,
        down=FLIGHT_DOWN_METERS,
        velocity=ORBIT_SPEED_METERS_PER_SEC,
    )
    await entry_task

    orbit_path = []
    for waypoint_index in range(1, ORBIT_WAYPOINT_COUNT + 1):
        angle = 2.0 * math.pi * waypoint_index / ORBIT_WAYPOINT_COUNT
        orbit_path.append(
            [
                center_north + ORBIT_RADIUS_METERS * math.cos(angle),
                center_east + ORBIT_RADIUS_METERS * math.sin(angle),
                FLIGHT_DOWN_METERS,
            ]
        )

    while True:
        move_task = await drone.move_on_path_async(
            path=orbit_path,
            velocity=ORBIT_SPEED_METERS_PER_SEC,
            yaw_control_mode=YawControlMode.ForwardOnly,
            yaw_is_rate=False,
            yaw=0.0,
        )
        await move_task


async def display_camera_and_command_gimbal(drone: Drone) -> None:
    """Pull and display RGB frames while periodically changing gimbal yaw."""
    window_name = "Gimbal FrontCamera RGB"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name, 800, 450)

    target_index = 0
    next_target_time = 0.0
    first_image_received = False

    while True:
        if time.monotonic() >= next_target_time:
            target_deg = GIMBAL_YAW_SEQUENCE_DEG[target_index]
            state = drone.get_gimbal_state(GIMBAL_ID)
            current_yaw = state["yaw"]
            target_yaw = math.radians(target_deg)
            yaw_error = target_yaw - current_yaw

            if math.isclose(yaw_error, 0.0, abs_tol=math.radians(0.1)):
                yaw_rate = 0.0
            else:
                yaw_rate = math.copysign(
                    math.radians(GIMBAL_RATE_DEG_PER_SEC), yaw_error
                )

            drone.set_gimbal_command(
                GIMBAL_ID,
                yaw=target_yaw,
                yaw_rate=yaw_rate,
            )
            projectairsim_log().info(
                "Gimbal target yaw %.1f deg (current %.1f deg)",
                target_deg,
                math.degrees(current_yaw),
            )

            target_index = (target_index + 1) % len(GIMBAL_YAW_SEQUENCE_DEG)
            next_target_time = time.monotonic() + GIMBAL_HOLD_SEC

        images = drone.get_images(CAMERA_ID, [ImageType.SCENE])
        image = images.get(ImageType.SCENE)
        if image is not None and len(image.get("data", b"")) > 0:
            cv2.imshow(window_name, unpack_image(image))
            if not first_image_received:
                projectairsim_log().info("FrontCamera RGB stream received")
                first_image_received = True

        key = cv2.waitKey(1) & 0xFF
        if key in (ord("q"), 27):
            projectairsim_log().info("Image window exit requested")
            return

        # Let asyncio process cancellation and completed API tasks.
        await asyncio.sleep(0)


async def main() -> None:
    client = ProjectAirSimClient()
    drone = None
    api_control_enabled = False
    armed = False
    orbit_task = None
    camera_task = None

    try:
        client.connect()
        world = World(
            client,
            scene_config_name = "scene_hello_drone_gimbal.jsonc",
            delay_after_load_sec=2,
            sim_config_path = "./sim_config"
        )
        drone = Drone(client, world, "Drone1")

        drone.enable_api_control()
        api_control_enabled = True
        drone.arm()
        armed = True

        camera_task = asyncio.create_task(
            display_camera_and_command_gimbal(drone)
        )
        projectairsim_log().info("FrontCamera display started")

        projectairsim_log().info("Taking off")
        takeoff_task = await drone.takeoff_async()
        await takeoff_task

        position = drone.get_ground_truth_kinematics()["pose"]["position"]
        projectairsim_log().info("Climbing to 20 m")
        climb_task = await drone.move_to_position_async(
            north=position["x"],
            east=position["y"],
            down=FLIGHT_DOWN_METERS,
            velocity=3.0,
        )
        await climb_task

        orbit_task = asyncio.create_task(
            fly_orbit(drone, position["x"], position["y"])
        )
        projectairsim_log().info(
            "Orbiting at 20 m; cycling the gimbal. Press q, Esc, or Ctrl+C to exit."
        )

        await camera_task

    except asyncio.CancelledError:
        projectairsim_log().info("Stop requested")
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
    finally:
        if camera_task is not None:
            camera_task.cancel()
            try:
                await camera_task
            except asyncio.CancelledError:
                pass
            except Exception as camera_err:
                projectairsim_log().warning(
                    f"Camera task stopped with an error: {camera_err}"
                )

        if orbit_task is not None:
            orbit_task.cancel()
            try:
                await orbit_task
            except asyncio.CancelledError:
                pass

        if drone is not None and api_control_enabled:
            try:
                drone.set_gimbal_command(GIMBAL_ID, yaw_rate=0.0)
                if armed:
                    projectairsim_log().info("Landing")
                    land_task = await drone.land_async()
                    await land_task
                    drone.disarm()
                drone.disable_api_control()
            except Exception as err:
                projectairsim_log().warning(f"Cleanup failed: {err}")

        cv2.destroyAllWindows()
        client.disconnect()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
