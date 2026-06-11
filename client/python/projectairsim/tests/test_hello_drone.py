"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Pytest end-end test script for hello_drone.py functionality
"""

import asyncio
import cv2
import os
import pytest
import time
import numpy as np

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.types import ImageType
from projectairsim.utils import projectairsim_log

from image_validation_utils import assert_rgb_scene_image_valid

# ---------------------------------------------------------------------------
# Optional reference image (PNG/JPG) for similarity regression on DownCamera RGB.
# Leave empty to skip reference correlation; set a path (or env override below)
# after capturing a baseline from this test in your Blocks environment.
# ---------------------------------------------------------------------------
HELLO_DRONE_REFERENCE_IMAGE_PATH: str = "hello_drone_ref_images"
HELLO_DRONE_IMAGE_SIMILARITY_MIN: float = 0.9

# Env overrides (optional): PROJECTAIRSIM_TEST_HELLO_DRONE_REF_IMAGE,
# PROJECTAIRSIM_TEST_HELLO_DRONE_SIM_MIN
HELLO_DRONE_REF_IMAGE = (
    os.environ.get("PROJECTAIRSIM_TEST_HELLO_DRONE_REF_IMAGE", "").strip()
    or HELLO_DRONE_REFERENCE_IMAGE_PATH.strip()
)
HELLO_DRONE_SIM_MIN = float(
    os.environ.get(
        "PROJECTAIRSIM_TEST_HELLO_DRONE_SIM_MIN", str(HELLO_DRONE_IMAGE_SIMILARITY_MIN)
    )
)
CONTEXT_TO_REF_STEP = {
    "initial": "initial",
    "Move-Up completed": "move_up",
    "Move-North1 completed": "move_north1",
    "Rotate-Z-60 completed": "rotate_z_60",
    "Move-North2 completed": "move_north2",
    "Rotate-Z-45a completed": "rotate_z_45a",
    "Move-West completed": "move_west",
    "Rotate-Z-45b completed": "rotate_z_45b",
}


def _resolve_context_reference_path(
    base_reference_path: str,
    context: str,
    default_extension: str,
    camera_suffix: str = "",
) -> str:
    base = (base_reference_path or "").strip()
    if not base:
        return ""

    step = CONTEXT_TO_REF_STEP.get(context)
    if not step:
        return base

    if os.path.isdir(base):
        candidate = os.path.join(
            base,
            f"hello_drone_{step}{camera_suffix}_ref{default_extension}",
        )
        return candidate if os.path.isfile(candidate) else base

    root, ext = os.path.splitext(base)
    candidate = f"{root}_{step}{ext or default_extension}"
    if os.path.isfile(candidate):
        return candidate

    parent = os.path.dirname(base) or "."
    canonical = os.path.join(
        parent,
        f"hello_drone_{step}{camera_suffix}_ref{default_extension}",
    )
    if os.path.isfile(canonical):
        return canonical

    return base


def _normalized_cross_correlation(a: np.ndarray, b: np.ndarray) -> float:
    if a.size == 0 or b.size == 0:
        return 0.0
    b_h, b_w = b.shape[:2]
    a_rs = cv2.resize(a, (b_w, b_h), interpolation=cv2.INTER_AREA).astype(np.float64)
    b_f = b.astype(np.float64)
    a_rs -= float(np.mean(a_rs))
    b_f -= float(np.mean(b_f))
    denom = float(np.linalg.norm(a_rs.ravel()) * np.linalg.norm(b_f.ravel()))
    if denom < 1e-9:
        return 0.0
    return float(np.dot(a_rs.ravel(), b_f.ravel()) / denom)


async def assert_scene_camera_similarity(drone, context: str) -> None:
    await asyncio.sleep(5.0)
    down_snaps = drone.get_images("DownCamera", [ImageType.SCENE])
    front_snaps = drone.get_images("FrontCamera", [ImageType.SCENE])

    down_scene = down_snaps[ImageType.SCENE]
    down_scene_ref_path = _resolve_context_reference_path(
        HELLO_DRONE_REF_IMAGE,
        context,
        ".png",
        "_down_camera",
    )
    assert_rgb_scene_image_valid(
        down_scene,
        reference_image_path=down_scene_ref_path,
        min_similarity_to_reference=HELLO_DRONE_SIM_MIN,
        min_gray_std=2.0,
    )

    front_scene = front_snaps[ImageType.SCENE]
    front_scene_ref_path = _resolve_context_reference_path(
        HELLO_DRONE_REF_IMAGE,
        context,
        ".png",
        "_front_camera",
    )
    assert_rgb_scene_image_valid(
        front_scene,
        reference_image_path=front_scene_ref_path,
        min_similarity_to_reference=HELLO_DRONE_SIM_MIN,
        min_gray_std=2.0,
    )


def _assert_rgb_message(img_msg, context: str) -> None:
    assert_rgb_scene_image_valid(
        img_msg,
        reference_image_path="",
        min_similarity_to_reference=HELLO_DRONE_SIM_MIN,
        min_gray_std=2.0,
    )


def check_image_rgb(img_msg):
    _assert_rgb_message(img_msg, "rgb_stream")


def check_imu(imu_msg):
    assert len(imu_msg) > 0
    orientation = imu_msg["orientation"]
    lin_accel = imu_msg["linear_acceleration"]
    ang_vel = imu_msg["angular_velocity"]
    assert -1.0 <= orientation["w"] <= 1.0
    assert -1.0 <= orientation["x"] <= 1.0
    assert -1.0 <= orientation["y"] <= 1.0
    assert -1.0 <= orientation["z"] <= 1.0
    assert -20.0 <= lin_accel["x"] <= 20.0
    assert -20.0 <= lin_accel["y"] <= 20.0
    assert -20.0 <= lin_accel["z"] <= 20.0
    assert -5.0 <= ang_vel["x"] <= 5.0
    assert -5.0 <= ang_vel["y"] <= 5.0
    assert -5.0 <= ang_vel["z"] <= 5.0


async def wait_for_pose_change(multirotor, prev_pose, timeout=2.0):
    start = time.time()
    while True:
        pose = multirotor.robot_actual_pose
        if pose is not None and pose != prev_pose:
            return pose
        if time.time() - start > timeout:
            pytest.fail("Timeout waiting for pose update")
        await asyncio.sleep(0.05)


@pytest.fixture(scope="class")
def multirotor():
    class ProjectAirSimTestObject:
        client = ProjectAirSimClient()
        client.connect()
        world = World(client, "scene_test_drone_hello_drone_refs.jsonc", 1)
        drone = Drone(client, world, "Drone1")
        robot_actual_pose = None

        def robot_actual_pose_callback(self, topic, message):
            self.robot_actual_pose = message

    multirotor_obj = ProjectAirSimTestObject()
    yield multirotor_obj

    print("\nTeardown client...")
    multirotor_obj.client.disconnect()


class TestClientBase:
    async def main(self, multirotor):
        print("start")
        drone = multirotor.drone
        client = multirotor.client
        world = multirotor.world

        client.subscribe(
            drone.robot_info["actual_pose"], multirotor.robot_actual_pose_callback
        )

        timeout = time.time() + 5
        multirotor.robot_actual_pose = None
        while multirotor.robot_actual_pose is None:
            if time.time() > timeout:
                pytest.fail("Timeout waiting for a pose message update")
            await asyncio.sleep(0.1)

        required_cameras = ["DownCamera", "FrontCamera"]
        missing_cameras = [cam for cam in required_cameras if cam not in drone.sensors]
        if missing_cameras:
            pytest.fail(
                "Missing required cameras in drone config: "
                f"{missing_cameras}. Ensure scene_test_drone_hello_drone_refs.jsonc is used."
            )

        client.subscribe(
            drone.sensors["DownCamera"]["scene_camera"],
            lambda _, rgb: check_image_rgb(rgb),
        )
        client.subscribe(
            drone.sensors["FrontCamera"]["scene_camera"],
            lambda _, rgb: check_image_rgb(rgb),
        )
        client.subscribe(
            drone.sensors["IMU1"]["imu_kinematics"],
            lambda _, imu: check_imu(imu),
        )
        await assert_scene_camera_similarity(drone, "initial")

        drone.enable_api_control()
        drone.arm()

        prev_pose = multirotor.robot_actual_pose

        # Move up
        move_up = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-2.0, duration=5.0
        )
        projectairsim_log().info("Move-Up invoked")
        await move_up
        projectairsim_log().info("Move-Up completed")
        await assert_scene_camera_similarity(drone, "Move-Up completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["z"] < prev_pose["position"]["z"]

        prev_pose = new_pose
        # Move north 1
        move_north1 = await drone.move_by_velocity_async(
            v_north=2.0, v_east=0.0, v_down=0.0, duration=5.0
        )
        projectairsim_log().info("Move-North1 invoked")
        await move_north1
        projectairsim_log().info("Move-North1 completed")
        await assert_scene_camera_similarity(drone, "Move-North1 completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["x"] > prev_pose["position"]["x"]

        prev_pose = new_pose
        # Rotate Z 60 degrees (rate 30 deg/s for 2s)
        import math
        await drone.rotate_by_yaw_rate_async(math.radians(30.0), 2.0)
        projectairsim_log().info("Rotate-Z-60 completed")
        await assert_scene_camera_similarity(drone, "Rotate-Z-60 completed")

        # Move north 2
        move_north2 = await drone.move_by_velocity_async(
            v_north=2.0, v_east=0.0, v_down=0.0, duration=5.0
        )
        projectairsim_log().info("Move-North2 invoked")
        await move_north2
        projectairsim_log().info("Move-North2 completed")
        await assert_scene_camera_similarity(drone, "Move-North2 completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["x"] > prev_pose["position"]["x"]

        prev_pose = new_pose
        # Rotate Z 45 degrees (a) (rate 22.5 deg/s for 2s)
        await drone.rotate_by_yaw_rate_async(math.radians(22.5), 2.0)
        projectairsim_log().info("Rotate-Z-45a completed")
        await assert_scene_camera_similarity(drone, "Rotate-Z-45a completed")

        # Move west
        move_west = await drone.move_by_velocity_async(
            v_north=0.0, v_east=-2.0, v_down=0.0, duration=5.0
        )
        projectairsim_log().info("Move-West invoked")
        await move_west
        projectairsim_log().info("Move-West completed")
        await assert_scene_camera_similarity(drone, "Move-West completed")
        new_pose = await wait_for_pose_change(multirotor, prev_pose)
        assert new_pose["position"]["y"] < prev_pose["position"]["y"]

        prev_pose = new_pose
        # Rotate Z 45 degrees (b) (rate 22.5 deg/s for 2s)
        await drone.rotate_by_yaw_rate_async(math.radians(22.5), 2.0)
        projectairsim_log().info("Rotate-Z-45b completed")
        await assert_scene_camera_similarity(drone, "Rotate-Z-45b completed")

        drone.disarm()
        drone.disable_api_control()

        client.disconnect()

    def test_hello_drone(self, multirotor):
        asyncio.run(self.main(multirotor))
