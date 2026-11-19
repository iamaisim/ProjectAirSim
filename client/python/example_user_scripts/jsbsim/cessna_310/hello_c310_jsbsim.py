"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Short demonstration of the standard JSBSim Cessna 310 model:
take off, turn toward a nearby waypoint, and land.
"""

import asyncio
import math
from pathlib import Path

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.image_utils import ImageDisplay
from projectairsim.utils import projectairsim_log


EARTH_RADIUS_M = 6_378_137.0
METERS_TO_FEET = 3.280839895
CRUISE_ALTITUDE_FT = 300.0
TURN_COMPLETION_RAD = math.radians(45.0)


class ProjectAirSimGroundedMonitor:
    """Detect grounded state from Project AirSim ground-truth kinematics."""

    def __init__(self, drone: Drone, ground_z_m: float):
        self._drone = drone
        self._ground_z_m = ground_z_m

    async def wait(self, timeout_sec: float) -> None:
        deadline = asyncio.get_running_loop().time() + timeout_sec
        stable_samples = 0
        while stable_samples < 5:
            kinematics = self._drone.get_ground_truth_kinematics()
            position_z_m = kinematics["pose"]["position"]["z"]
            vertical_velocity_mps = kinematics["twist"]["linear"]["z"]

            # NED z increases downward. Require the Project AirSim pose to be
            # back at its initial ground level and vertically stationary for
            # one second before declaring the aircraft grounded.
            at_ground_level = position_z_m >= self._ground_z_m - 0.25
            vertically_still = abs(vertical_velocity_mps) <= 0.1
            stable_samples = (
                stable_samples + 1 if at_ground_level and vertically_still else 0
            )

            if asyncio.get_running_loop().time() >= deadline:
                raise TimeoutError(
                    f"Timed out after {timeout_sec}s waiting for Project AirSim "
                    "ground-truth kinematics to report grounded"
                )
            await asyncio.sleep(0.2)


def set_properties(drone: Drone, properties: dict[str, float]) -> None:
    """Set a group of JSBSim properties and fail if one is rejected."""
    for property_name, value in properties.items():
        if not drone.set_jsbsim_property(property_name, value):
            raise RuntimeError(f"Could not set JSBSim property {property_name}")


async def wait_for_property_at_least(
    drone: Drone, property_name: str, threshold: float, timeout_sec: float
) -> None:
    deadline = asyncio.get_running_loop().time() + timeout_sec
    while drone.get_jsbsim_property(property_name) < threshold:
        if asyncio.get_running_loop().time() >= deadline:
            raise TimeoutError(
                f"Timed out after {timeout_sec}s waiting for "
                f"{property_name} >= {threshold}"
            )
        await asyncio.sleep(0.1)


async def set_after_property_reaches(
    drone: Drone,
    watched_property: str,
    threshold: float,
    property_name: str,
    value: float,
    timeout_sec: float,
) -> None:
    await wait_for_property_at_least(
        drone, watched_property, threshold, timeout_sec
    )
    set_properties(drone, {property_name: value})


async def takeoff(drone: Drone, timeout_sec: float = 90.0) -> None:
    """Start the engines and perform the standard takeoff sequence."""
    set_properties(
        drone,
        {
            "fcs/left-brake-cmd-norm": 0.0,
            "fcs/right-brake-cmd-norm": 0.0,
            "fcs/center-brake-cmd-norm": 0.0,
            "gear/gear-cmd-norm": 1.0,
            "fcs/mixture-cmd-norm[0]": 1.0,
            "fcs/mixture-cmd-norm[1]": 1.0,
            "fcs/advance-cmd-norm[0]": 1.0,
            "fcs/advance-cmd-norm[1]": 1.0,
            "propulsion/magneto_cmd": 3.0,
            "fcs/throttle-cmd-norm[0]": 1.0,
            "fcs/throttle-cmd-norm[1]": 1.0,
            "propulsion/starter_cmd": 1.0,
            "ap/altitude_setpoint": CRUISE_ALTITUDE_FT,
            "ap/attitude_hold": 0.0,
            "ap/heading_setpoint": 0.0,
            "ap/heading-setpoint-select": 0.0,
            "ap/heading_hold": 1.0,
            "ap/active-waypoint": 0.0,
        },
    )

    await wait_for_property_at_least(
        drone, "simulation/sim-time-sec", 0.25, timeout_sec
    )

    await asyncio.gather(
        set_after_property_reaches(
            drone,
            "velocities/vc-fps",
            145.0,
            "ap/altitude_hold",
            1.0,
            timeout_sec,
        ),
        set_after_property_reaches(
            drone,
            "position/h-agl-ft",
            40.0,
            "gear/gear-cmd-norm",
            0.0,
            timeout_sec,
        ),
    )
    await wait_for_property_at_least(
        drone, "position/h-agl-ft", 100.0, timeout_sec
    )


def nearby_waypoint(
    drone: Drone, north_offset_m: float, east_offset_m: float
) -> tuple[float, float]:
    """Create a nearby waypoint from the aircraft's current JSBSim position."""
    latitude_rad = drone.get_jsbsim_property("position/lat-gc-rad")
    longitude_rad = drone.get_jsbsim_property("position/long-gc-rad")
    return (
        latitude_rad + north_offset_m / EARTH_RADIUS_M,
        longitude_rad
        + east_offset_m / (EARTH_RADIUS_M * math.cos(latitude_rad)),
    )


async def fly_to_nearby_waypoint(
    drone: Drone,
    north_offset_m: float,
    east_offset_m: float,
    timeout_sec: float,
) -> None:
    """Turn at least 45 degrees toward a waypoint near the aircraft."""
    initial_heading_rad = drone.get_jsbsim_property("attitude/heading-true-rad")
    target_latitude_rad, target_longitude_rad = nearby_waypoint(
        drone, north_offset_m, east_offset_m
    )
    set_properties(
        drone,
        {
            "ap/altitude_setpoint": CRUISE_ALTITUDE_FT,
            "guidance/target_wp_latitude_rad": target_latitude_rad,
            "guidance/target_wp_longitude_rad": target_longitude_rad,
            "ap/heading-setpoint-select": 1.0,
            "ap/heading_hold": 1.0,
            "ap/altitude_hold": 1.0,
            "ap/active-waypoint": 1.0,
            "fcs/throttle-cmd-norm[0]": 0.65,
            "fcs/throttle-cmd-norm[1]": 0.65,
        },
    )

    # Wait for GNCUtilities to replace its previous waypoint distance.
    await wait_for_property_at_least(
        drone, "guidance/wp-distance", 100.0 * METERS_TO_FEET, 10.0
    )
    initial_distance_ft = drone.get_jsbsim_property("guidance/wp-distance")

    deadline = asyncio.get_running_loop().time() + timeout_sec
    while True:
        current_distance_ft = drone.get_jsbsim_property("guidance/wp-distance")
        current_heading_rad = drone.get_jsbsim_property("attitude/heading-true-rad")
        heading_change_rad = abs(
            (current_heading_rad - initial_heading_rad + math.pi)
            % (2.0 * math.pi)
            - math.pi
        )
        if (
            heading_change_rad >= TURN_COMPLETION_RAD
            and current_distance_ft < initial_distance_ft
        ):
            return
        if asyncio.get_running_loop().time() >= deadline:
            raise TimeoutError(
                f"Timed out after {timeout_sec}s turning toward nearby waypoint"
            )
        await asyncio.sleep(0.2)


async def land(
    drone: Drone,
    grounded_monitor: ProjectAirSimGroundedMonitor,
    timeout_sec: float = 120.0,
) -> None:
    """Descend and brake only after Project AirSim reports grounded contact."""
    current_heading_deg = math.degrees(
        drone.get_jsbsim_property("attitude/heading-true-rad")
    )
    terrain_altitude_ft = drone.get_jsbsim_property(
        "position/terrain-elevation-asl-ft"
    )

    set_properties(
        drone,
        {
            "ap/altitude_setpoint": terrain_altitude_ft,
            "ap/altitude_hold": 1.0,
            "ap/heading_setpoint": current_heading_deg,
            "ap/heading-setpoint-select": 0.0,
            "ap/heading_hold": 1.0,
            "gear/gear-cmd-norm": 1.0,
            "fcs/flap-cmd-norm": 3.0,
            "fcs/throttle-cmd-norm[0]": 0.18,
            "fcs/throttle-cmd-norm[1]": 0.18,
            "fcs/elevator-cmd-norm": 0.0,
        },
    )

    projectairsim_log().info(
        "Waiting for Project AirSim to report the aircraft grounded"
    )
    await grounded_monitor.wait(timeout_sec)

    # Do not command JSBSim brakes before Project AirSim reports ground contact.
    set_properties(
        drone,
        {
            "ap/altitude_hold": 0.0,
            "ap/heading_hold": 0.0,
            "fcs/throttle-cmd-norm[0]": 0.0,
            "fcs/throttle-cmd-norm[1]": 0.0,
            "fcs/left-brake-cmd-norm": 1.0,
            "fcs/right-brake-cmd-norm": 1.0,
            "fcs/center-brake-cmd-norm": 1.0,
        },
    )


async def main():
    client = ProjectAirSimClient()
    image_display = ImageDisplay()

    try:
        client.connect()

        sim_config_path = str(Path(__file__).resolve().parent / "sim_config")
        world = World(
            client,
            "scene_c310_jsbsim.jsonc",
            delay_after_load_sec=2,
            sim_config_path=f"{sim_config_path}/",
        )
        drone = Drone(client, world, "c310")

        chase_cam_window = "ChaseCam"
        image_display.add_chase_cam(chase_cam_window)
        client.subscribe(
            drone.sensors["Chase"]["scene_camera"],
            lambda _, chase: image_display.receive(chase, chase_cam_window),
        )

        grounded_monitor = ProjectAirSimGroundedMonitor(
            drone,
            drone.get_ground_truth_pose()["translation"]["z"],
        )
        image_display.start()

        projectairsim_log().info("Short C310 demo: takeoff")
        await takeoff(drone)

        # A waypoint 1.5 km to the right forces a visible turn while remaining
        # close enough to keep the demonstration short.
        projectairsim_log().info("Short C310 demo: turn to nearby waypoint")
        await fly_to_nearby_waypoint(
            drone,
            north_offset_m=0.0,
            east_offset_m=1_500.0,
            timeout_sec=60.0,
        )

        projectairsim_log().info("Short C310 demo: landing")
        await land(drone, grounded_monitor)
        projectairsim_log().info("Short C310 demo: completed")

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.disconnect()
        image_display.stop()


if __name__ == "__main__":
    asyncio.run(main())
