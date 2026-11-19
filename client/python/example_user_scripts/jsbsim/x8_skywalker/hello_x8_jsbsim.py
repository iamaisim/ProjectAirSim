"""Run the Skywalker X8 JSBSim example with a paused steppable scene."""

from __future__ import annotations

import argparse
import math
from pathlib import Path

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log

from x8_autopilot import WaypointFollower, X8Autopilot


SCRIPT_DIR = Path(__file__).resolve().parent
CONFIG_DIR = SCRIPT_DIR / "sim_config"
SCENE = "scene_x8_jsbsim.jsonc"
CONTROL_PERIOD_SEC = 0.02
CONTROL_PERIOD_NS = int(CONTROL_PERIOD_SEC * 1e9)
CRUISE_AIRSPEED_KTS = 42.0
CRUISE_ALTITUDE_FT = 420.0
WAYPOINTS = [
    (500.0, 0.0, CRUISE_ALTITUDE_FT),
    (500.0, 500.0, CRUISE_ALTITUDE_FT),
    (0.0, 500.0, CRUISE_ALTITUDE_FT),
    (0.0, 0.0, CRUISE_ALTITUDE_FT),
]


def seed_air_launch(drone: Drone) -> None:
    """Initialize the paused X8 inside JSBSim's own state space.

    Updating Project AirSim ground-truth kinematics bypasses JSBSim and leaves
    its geodetic/inertial state inconsistent.  Seed body-axis velocity and
    angular rates through JSBSim properties instead.
    """
    initial_properties = {
        "velocities/u-fps": CRUISE_AIRSPEED_KTS * 1.687809857,
        "velocities/v-fps": 0.0,
        "velocities/w-fps": 0.0,
        "velocities/p-rad_sec": 0.0,
        "velocities/q-rad_sec": 0.0,
        "velocities/r-rad_sec": 0.0,
        "fcs/elevator-cmd-norm": 0.0,
        "fcs/aileron-cmd-norm": 0.0,
        "fcs/throttle-cmd-norm": 0.65,
    }
    for name, value in initial_properties.items():
        if not drone.set_jsbsim_property(name, value):
            raise RuntimeError(f"JSBSim rejected initial property '{name}'")


def ensure_finite_state(autopilot: X8Autopilot) -> None:
    """Stop before invalid JSBSim state reaches Unreal/LWC transforms."""
    state_names = (
        "position/lat-geod-deg",
        "position/long-gc-deg",
        "position/h-sl-ft",
        "velocities/vtrue-kts",
        "attitude/phi-rad",
        "attitude/theta-rad",
    )
    state = {name: autopilot.get(name) for name in state_names}
    if not all(math.isfinite(value) for value in state.values()):
        raise RuntimeError(f"JSBSim produced a non-finite state: {state}")
    if not (-90.0 <= state["position/lat-geod-deg"] <= 90.0):
        raise RuntimeError(f"JSBSim latitude escaped its valid range: {state}")
    if not (-180.0 <= state["position/long-gc-deg"] <= 180.0):
        raise RuntimeError(f"JSBSim longitude escaped its valid range: {state}")


def main(max_sim_time_sec: float) -> None:
    client = ProjectAirSimClient()
    connected = False
    try:
        client.connect()
        connected = True
        world = World(
            client,
            SCENE,
            delay_after_load_sec=0,
            sim_config_path=str(CONFIG_DIR),
        )
        drone = Drone(client, world, "X8")
        seed_air_launch(drone)

        autopilot = X8Autopilot(drone, CONTROL_PERIOD_SEC)
        autopilot.set_controls(elevator=0.0, aileron=0.0, throttle=1.0)
        follower = WaypointFollower(autopilot, WAYPOINTS)
        iterations = int(max_sim_time_sec / CONTROL_PERIOD_SEC)

        for iteration in range(iterations):
            ensure_finite_state(autopilot)
            complete = follower.update(CRUISE_AIRSPEED_KTS)
            world.continue_for_sim_time(CONTROL_PERIOD_NS)
            if iteration % 50 == 0:
                projectairsim_log().info(
                    f"X8 t={iteration * CONTROL_PERIOD_SEC:.1f}s "
                    f"wp={follower.index + 1} "
                    f"alt={autopilot.get('position/h-sl-ft'):.1f}ft "
                    f"tas={autopilot.get('velocities/vtrue-kts'):.1f}kt"
                )
            if complete:
                projectairsim_log().info("X8 waypoint circuit completed")
                break
        else:
            projectairsim_log().info(
                f"X8 reached the configured {max_sim_time_sec:.1f}s simulation limit"
            )
    finally:
        if connected:
            client.disconnect()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--max-sim-time", type=float, default=20.0)
    args = parser.parse_args()
    main(args.max_sim_time)
