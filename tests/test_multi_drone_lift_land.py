"""Multi-drone lift-land test using nav-jax RL robot config (3 cameras per drone).

Loads the multi-drone scene with the RL robot config (Chase, World, FPV cameras),
arms N drones, lifts them, verifies altitude, then lands.

Run from: ProjectAirSim repo root
Requires: UE editor with NavGym map in Play mode.

Usage:
    uv run pytest tests/test_multi_drone_lift_land.py -v
    uv run pytest tests/test_multi_drone_lift_land.py -v -k "test_lift_land_3"
"""

import sys
import time
from pathlib import Path

import commentjson
import pytest
from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.types import ImageType
from projectairsim.types import Pose
from projectairsim.utils import load_scene_config_as_dict

SIM_ADDRESS = "172.23.240.1"
STEP_NS = 3_000_000  # 3ms per physics tick

# nav-jax sim_config directory (has robot + scene configs)
NAV_JAX_SIM_CONFIG = str(Path(__file__).resolve().parent.parent.parent / "nav-jax" / "sims" / "airsim" / "sim_config")
SCENE_CONFIG = "scene_gate_course_rl_multi.jsonc"


# ── Helpers ──────────────────────────────────────────────────────────


def load_scene(client, num_drones):
    """Load multi-drone scene and return World + list of drone topics."""
    world = World(
        client,
        SCENE_CONFIG,
        sim_config_path=NAV_JAX_SIM_CONFIG,
        delay_after_load_sec=2,
    )

    drone_topics = [f"{world.parent_topic}/robots/Drone{i+1}" for i in range(num_drones)]
    return world, drone_topics


def arm_drone(client, topic):
    """EnableApiControl + Arm + SetAcroMode."""
    client.request({"method": f"{topic}/EnableApiControl", "params": {}, "version": 1.0})
    client.request({"method": f"{topic}/Arm", "params": {}, "version": 1.0})
    client.request({"method": f"{topic}/SetAcroMode", "params": {"enabled": True}, "version": 1.0})


def set_controls(client, topic, throttle, roll_rate=0.0, pitch_rate=0.0, yaw_rate=0.0):
    client.request({
        "method": f"{topic}/SetAngleRatesThrottle",
        "params": {"roll_rate": roll_rate, "pitch_rate": pitch_rate,
                   "yaw_rate": yaw_rate, "throttle": throttle},
        "version": 1.0,
    })


# ── Tests ────────────────────────────────────────────────────────────


def _run_lift_land(num_drones):
    """Core lift-land test for N drones."""
    client = ProjectAirSimClient(address=SIM_ADDRESS)
    client.connect()

    try:
        world, drone_topics = load_scene(client, num_drones)
        drone_names = [f"Drone{i+1}" for i in range(num_drones)]

        # Arm all drones
        for topic in drone_topics:
            arm_drone(client, topic)

        # Step once to settle
        resp = world.step(STEP_NS)
        assert "drones" in resp
        for name in drone_names:
            assert name in resp["drones"], f"{name} not in step response"

        # Record starting z for each drone
        start_z = {}
        for name in drone_names:
            start_z[name] = resp["drones"][name]["state"]["position"]["z"]
            print(f"  {name} start z={start_z[name]:.2f}")

        # Phase 1: Gentle climb for ~3s
        for topic in drone_topics:
            set_controls(client, topic, throttle=0.5)

        for _ in range(1000):
            resp = world.step(STEP_NS)

        print("\nAfter climb:")
        for name in drone_names:
            z = resp["drones"][name]["state"]["position"]["z"]
            print(f"  {name} z={z:.2f}")
            assert z < start_z[name] - 0.5, (
                f"{name} didn't climb: start={start_z[name]:.2f}, now={z:.2f}"
            )

        # Phase 2: Pitch forward for ~2s
        print("\nPitching forward...")
        for topic in drone_topics:
            set_controls(client, topic, throttle=0.4, pitch_rate=0.5)

        for _ in range(666):
            resp = world.step(STEP_NS)

        # Phase 3: Yaw spin for ~2s
        print("Yawing...")
        for topic in drone_topics:
            set_controls(client, topic, throttle=0.4, yaw_rate=1.0)

        for _ in range(666):
            resp = world.step(STEP_NS)

        # Phase 4: Roll tilt for ~2s
        print("Rolling...")
        for topic in drone_topics:
            set_controls(client, topic, throttle=0.4, roll_rate=0.5)

        for _ in range(666):
            resp = world.step(STEP_NS)

        # Phase 5: Hover for ~2s to settle
        print("Hovering...")
        for topic in drone_topics:
            set_controls(client, topic, throttle=0.35)

        for _ in range(666):
            resp = world.step(STEP_NS)

        # Phase 6: Cut throttle — drones descend for ~3s
        print("Descending...")
        for topic in drone_topics:
            set_controls(client, topic, throttle=0.0)

        climb_z = {name: resp["drones"][name]["state"]["position"]["z"] for name in drone_names}

        for _ in range(1000):
            resp = world.step(STEP_NS)

        print("\nAfter descend:")
        for name in drone_names:
            z = resp["drones"][name]["state"]["position"]["z"]
            print(f"  {name} z={z:.2f}")
            assert z > climb_z[name], (
                f"{name} didn't descend: climb={climb_z[name]:.2f}, now={z:.2f}"
            )

        # Phase 3: Get images from FPV camera on each drone
        print("\nFPV images:")
        for i, name in enumerate(drone_names):
            drone = Drone(client, world, name)
            images = drone.get_images(camera_id="FPV", image_type_ids=[ImageType.SCENE])
            img = images.get(ImageType.SCENE)
            assert img is not None, f"{name}: FPV image is None"
            assert img["width"] == 64, f"{name}: expected 64px, got {img['width']}"
            assert img["height"] == 64, f"{name}: expected 64px, got {img['height']}"
            assert len(img["data"]) == 64 * 64 * 3, f"{name}: wrong data size"
            print(f"  {name}: FPV {img['width']}x{img['height']} OK")

        print(f"\n✓ {num_drones}-drone lift-land + FPV capture passed")

    finally:
        client.disconnect()


def test_lift_land_1():
    _run_lift_land(1)


def test_lift_land_3():
    _run_lift_land(3)


def test_lift_land_10():
    _run_lift_land(10)
