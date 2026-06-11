"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
End-to-end tests for ProjectAirSim Services, request-response APIs
"""

import asyncio
from datetime import datetime
import math
import time
from typing import Dict, Tuple

import numpy as np
from pynng import NNGException
import pytest

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.image_utils import segmentation_id_to_color, segmentation_color_to_id
from projectairsim.utils import (
    geo_to_ned_coordinates,
    quaternion_to_rpy,
    unpack_image,
)
from projectairsim.types import (
    Pose,
    Vector3,
    Quaternion,
    WeatherParameter,
    ImageType,
    BoxAlignment,
    Color,
    LandedState,
)


def _bgr_center_mean(image_msg: Dict) -> np.ndarray:
    """Mean BGR in a center crop of a camera image message."""
    bgr = unpack_image(image_msg)
    h, w = bgr.shape[:2]
    cy, cx = h // 2, w // 2
    y0, y1 = max(0, cy - 24), min(h, cy + 24)
    x0, x1 = max(0, cx - 24), min(w, cx + 24)
    patch = bgr[y0:y1, x0:x1]
    return patch.mean(axis=(0, 1)).astype(np.float64)


def _bgr_object_mean(image_msg: Dict, object_name: str) -> np.ndarray:
    """Mean BGR inside the annotated bbox for a given object, if available."""
    bgr = unpack_image(image_msg)
    annotations = image_msg.get("annotations") or []

    for annotation in annotations:
        if annotation.get("object_id") != object_name:
            continue

        bbox = annotation.get("bbox2d") or {}
        center = bbox.get("center") or {}
        size = bbox.get("size") or {}

        cx = int(round(center.get("x", bgr.shape[1] / 2)))
        cy = int(round(center.get("y", bgr.shape[0] / 2)))
        half_w = max(4, int(round(size.get("x", 0) / 2.0)))
        half_h = max(4, int(round(size.get("y", 0) / 2.0)))

        x0 = max(0, cx - half_w)
        x1 = min(bgr.shape[1], cx + half_w)
        y0 = max(0, cy - half_h)
        y1 = min(bgr.shape[0], cy + half_h)

        patch = bgr[y0:y1, x0:x1]
        if patch.size > 0:
            return patch.mean(axis=(0, 1)).astype(np.float64)

    return _bgr_center_mean(image_msg)


def _capture_object_mean(
    drone: Drone,
    world: World,
    object_name: str,
    camera_id: str = "DownCamera",
    settle_sec: float = 0.3,
) -> np.ndarray:
    drone.camera_look_at_object(camera_id=camera_id, object_name=object_name)
    time.sleep(settle_sec)

    world.pause()
    try:
        image = drone.get_images(camera_id, [ImageType.SCENE])[ImageType.SCENE]
    finally:
        world.resume()

    return _bgr_object_mean(image, object_name)


def _quat_dict_rpy(q: Dict[str, float]) -> Tuple[float, float, float]:
    return quaternion_to_rpy(q["w"], q["x"], q["y"], q["z"])


def _make_pose(x: float, y: float, z: float) -> Pose:
    translation = Vector3({"x": x, "y": y, "z": z})
    rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
    return Pose({"translation": translation, "rotation": rotation, "frame_id": "DEFAULT_ID"})


def _pose_from_dict(pose_dict: Dict) -> Pose:
    return Pose(
        {
            "translation": Vector3(pose_dict["translation"]),
            "rotation": Quaternion(pose_dict["rotation"]),
            "frame_id": "DEFAULT_ID",
        }
    )


def _reset_scene_drone_to_initial(scene_drone: Drone, initial_pose_dict: Dict) -> None:
    # Reset control state first so set_pose is deterministic across tests.
    scene_drone.disarm()
    scene_drone.disable_api_control()
    scene_drone.set_pose(_pose_from_dict(initial_pose_dict))


def _ensure_shared_scene_active(scene_world: World, scene_drone: Drone) -> None:
    """Reload shared scene only when another test switched away from it."""
    try:
        scene_world.get_sim_clock_type()
        scene_drone.get_ground_truth_pose()
        return
    except Exception:
        pass

    scene_world.load_scene(scene_world.sim_config, delay_after_load_sec=1.0)
    scene_drone.set_topics(scene_world)
    scene_drone.home_geo_point = scene_world.home_geo_point


@pytest.fixture(scope="module", autouse=True)
def client(request) -> ProjectAirSimClient:
    client = ProjectAirSimClient()
    try:
        client.connect()
    except NNGException as err:
        err_msg = (
            f"ProjectAirSim client connection failed with reason:{str(err)}\n"
            f"Is the ProjectAirSim server running?"
        )
        raise Exception(err_msg)

    def disconnect():
        client.disconnect()

    request.addfinalizer(disconnect)
    return client


@pytest.fixture(scope="module")
def scene_world(client):
    return World(client, "scene_test_drone.jsonc", delay_after_load_sec=1)


@pytest.fixture(scope="module")
def scene_drone(client, scene_world):
    return Drone(client, scene_world, "Drone1")


INITIAL_SCENE_DRONE_POSE = {
    "translation": {"x": 0.0, "y": 0.0, "z": -4.0},
    "rotation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
}


@pytest.fixture(scope="module")
def initial_scene_drone_pose():
    return INITIAL_SCENE_DRONE_POSE


@pytest.fixture(scope="function")
def reset_scene_drone(scene_world, scene_drone, initial_scene_drone_pose):
    _ensure_shared_scene_active(scene_world, scene_drone)
    _reset_scene_drone_to_initial(scene_drone, initial_scene_drone_pose)
    time.sleep(2.0)
    return scene_drone


def test_enable_disable_api_control(client, scene_drone):
    try:
        drone = scene_drone
        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        api_control_reported = drone.is_api_control_enabled()
        assert api_control_reported is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
        api_control_reported = drone.is_api_control_enabled()
        assert api_control_reported is False
    except NNGException as err:
        raise Exception(str(err))


def test_arm_disarm(client, scene_drone):
    try:
        drone = scene_drone
        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def takeoff_and_land_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    land = await drone.land_async()
    await land


def test_takeoff_and_land_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(takeoff_and_land_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def takeoff_and_hover_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    hover = await drone.hover_async()
    await hover


def test_takeoff_and_hover_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(takeoff_and_hover_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_by_velocity_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_north_up = await drone.move_by_velocity_async(
        v_north=2.0, v_east=0.0, v_down=-1.0, duration=2.0
    )
    await move_north_up


def test_move_by_velocity_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_by_velocity_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_by_velocity_z_async(drone):
    z0 = drone.get_ground_truth_pose()["translation"]["z"]
    z0_gt = INITIAL_SCENE_DRONE_POSE["translation"]["z"]
    assert z0 == pytest.approx(z0_gt, abs=0.5)
    move_at_z = await drone.move_by_velocity_z_async(
        v_north=2.0, v_east=0.0, z=-5.0, duration=2.0
    )
    await move_at_z
    z1 = drone.get_ground_truth_pose()["translation"]["z"]
    assert z1 == pytest.approx(-5.0, abs=1.0)
    assert z1 <= z0 + 1.0


def test_move_by_velocity_z_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_by_velocity_z_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_by_velocity_body_frame_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_north_up = await drone.move_by_velocity_body_frame_async(
        v_forward=2.0, v_right=0.0, v_down=-1.0, duration=2.0
    )
    await move_north_up


def test_move_by_velocity_body_frame_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_by_velocity_body_frame_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_by_velocity_body_frame_z_async(drone):
    z0 = drone.get_ground_truth_pose()["translation"]["z"]
    move_at_z = await drone.move_by_velocity_body_frame_z_async(
        v_forward=2.0, v_right=0.0, z=-5.0, duration=2.0
    )
    await move_at_z
    z1 = drone.get_ground_truth_pose()["translation"]["z"]
    assert z1 == pytest.approx(-5.0, abs=4.0)
    assert z1 <= z0 + 0.5


def test_move_by_velocity_body_frame_z_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_by_velocity_body_frame_z_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_by_heading_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_by_heading = await drone.move_by_heading_async(heading=90.0, speed=5.0)
    await move_by_heading


def test_move_by_heading_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_by_heading_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_to_position_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_to_pos = await drone.move_to_position_async(
        north=3.0, east=3.0, down=-6.0, velocity=1.0
    )
    await move_to_pos


def test_move_to_position_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_to_position_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_to_geo_position_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_to_pos = await drone.move_to_geo_position_async(
        latitude=47.641460, longitude=-122.140160, altitude=125.0, velocity=1.0
    )
    await move_to_pos


def test_move_to_geo_position_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_to_geo_position_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def go_home_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_north_up = await drone.move_by_velocity_async(
        v_north=2.0, v_east=0.0, v_down=-1.0, duration=2.0
    )
    await move_north_up

    go_home = await drone.go_home_async()
    await go_home


def test_set_object_pose(client, scene_world):
    try:
        object_name = "TempPoseCube"
        asset_name = "1M_Cube"
        original_pose = _make_pose(20, 0, -5)
        updated_pose = _make_pose(25, 0, -5)
        scale = [3.0, 3.0, 3.0]

        scene_world.spawn_object(object_name, asset_name, original_pose, scale, False)

        status = scene_world.set_object_pose(object_name, updated_pose, True)
        print("Set pose success:", status)
        assert status
        time.sleep(0.5)
        obtained_pose = scene_world.get_object_pose(object_name)
        assert obtained_pose == updated_pose
        scene_world.set_object_pose(object_name, original_pose, True)
        scene_world.destroy_object(object_name)
    except NNGException as err:
        raise Exception(str(err))


def test_set_object_scale(client, scene_world):
    try:
        object_name = "TempScaleCube"
        asset_name = "1M_Cube"
        pose = _make_pose(20, 0, -5)
        original_scale = [1.0, 1.0, 1.0]
        updated_scale = [10.0, 10.0, 10.0]

        scene_world.spawn_object(object_name, asset_name, pose, original_scale, False)

        status = scene_world.set_object_scale(object_name, updated_scale)
        print("Set scale success:", status)
        assert status
        new_scale = scene_world.get_object_scale(object_name)
        assert new_scale == updated_scale
        scene_world.destroy_object(object_name)

    except NNGException as err:
        raise Exception(str(err))


async def move_on_geo_path_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    path = [[47.641460, -122.140160, 125.0], [47.641470, -122.140165, 126.0]]
    move_on_path = await drone.move_on_geo_path_async(path, velocity=1.0)
    await move_on_path


def test_move_on_geo_path_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_on_geo_path_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def rotate_to_yaw_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    rotate = await drone.rotate_to_yaw_async(yaw=3.14)
    await rotate


def test_rotate_yaw_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(rotate_to_yaw_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def rotate_by_yaw_rate_async(drone):
    take_off = await drone.takeoff_async()
    await take_off

    pose0 = drone.get_ground_truth_pose()
    r0 = _quat_dict_rpy(pose0["rotation"])
    rotate = await drone.rotate_by_yaw_rate_async(yaw_rate=0.35, duration=2.5)
    await rotate
    pose1 = drone.get_ground_truth_pose()
    r1 = _quat_dict_rpy(pose1["rotation"])
    dyaw = (r1[2] - r0[2] + math.pi) % (2 * math.pi) - math.pi
    assert abs(dyaw) > 0.05


def test_rotate_by_yaw_rate_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(rotate_by_yaw_rate_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


def test_get_sim_clock_type(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        assert "unknown" not in clock_type
    except NNGException as err:
        raise Exception(str(err))


def test_get_sim_time(client, scene_world):
    try:
        sim_time = scene_world.get_sim_time()
        assert sim_time is not None
        print(f"Received: sim_time={sim_time * 1e-9} seconds")
    except NNGException as err:
        raise Exception(str(err))


def test_pause_resume(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        sim_pause_resume_state: str = scene_world.pause()
        sim_time = scene_world.get_sim_time()
        time.sleep(0.1)
        if "steppable" in clock_type:
            assert "paused" in sim_pause_resume_state
            assert scene_world.get_sim_time() == sim_time
        elif "real-time" in clock_type:
            assert "WARNING" in sim_pause_resume_state
            assert scene_world.get_sim_time() > sim_time
        elif "engine-driven" in clock_type:
            assert "WARNING" in sim_pause_resume_state

        sim_pause_resume_state: str = scene_world.resume()
        if "steppable" in clock_type:
            assert "resumed" in sim_pause_resume_state
        elif "real-time" in clock_type:
            assert "WARNING" in sim_pause_resume_state
        elif "engine-driven" in clock_type:
            assert "WARNING" in sim_pause_resume_state
        sim_time = scene_world.get_sim_time()
        time.sleep(0.1)
        assert scene_world.get_sim_time() > sim_time
    except NNGException as err:
        raise Exception(str(err))


def test_is_paused(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        scene_world.pause()
        if "steppable" in clock_type:
            assert scene_world.is_paused() is True
        elif "real-time" in clock_type:
            assert scene_world.is_paused() is False
        elif "engine-driven" in clock_type:
            assert scene_world.is_paused() is False

        scene_world.resume()
        assert scene_world.is_paused() is False
    except NNGException as err:
        raise Exception(str(err))


def test_continue_for_sim_time(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        scene_world.pause()
        sim_time = scene_world.get_sim_time()
        delta_time = 5 * 3e6  # TODO Has to be a multiple of JSON step ns
        scene_world.continue_for_sim_time(delta_time)
        if "steppable" in clock_type:
            assert scene_world.get_sim_time() == sim_time + delta_time
        elif "real-time" in clock_type:
            assert scene_world.get_sim_time() >= sim_time
        elif "engine-driven" in clock_type:
            # engine-driven doesn't support continue_for_sim_time
            pass
        scene_world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_continue_until_sim_time(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        scene_world.pause()
        sim_time = scene_world.get_sim_time()
        target_time = sim_time + 5 * 3e6  # TODO Has to be a multiple of JSON step ns
        scene_world.continue_until_sim_time(target_time)
        if "steppable" in clock_type:
            assert scene_world.get_sim_time() == target_time
        elif "real-time" in clock_type:
            assert scene_world.get_sim_time() >= target_time
        elif "engine-driven" in clock_type:
            # engine-driven doesn't support continue_until_sim_time
            pass
        scene_world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_continue_for_n_steps(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        scene_world.pause()
        scene_world.continue_for_n_steps(1)  # Step to next multiple of the step size
        sim_time = scene_world.get_sim_time()
        step_time = 3e6  # TODO This is JSON config dependent
        scene_world.continue_for_n_steps(1)
        if "steppable" in clock_type:
            assert scene_world.get_sim_time() == sim_time + step_time * 1
        elif "real-time" in clock_type:
            assert scene_world.get_sim_time() >= sim_time

        sim_time = scene_world.get_sim_time()
        scene_world.continue_for_n_steps(3)
        if "steppable" in clock_type:
            assert scene_world.get_sim_time() == sim_time + step_time * 3
        elif "real-time" in clock_type:
            assert scene_world.get_sim_time() >= sim_time
        scene_world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_continue_for_single_step(client, scene_world):
    try:
        clock_type = scene_world.get_sim_clock_type()
        scene_world.pause()
        scene_world.continue_for_single_step()  # Step to next multiple of the step size
        sim_time = scene_world.get_sim_time()
        step_time = 3e6  # TODO This is JSON config dependent
        scene_world.continue_for_single_step()
        if "steppable" in clock_type:
            assert scene_world.get_sim_time() == sim_time + step_time * 1
        elif "real-time" in clock_type:
            assert scene_world.get_sim_time() >= sim_time
        scene_world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_list_actors(client, scene_world, scene_drone):
    try:
        actor_ids = scene_world.list_actors()
        print(f"Received actor_ids: {actor_ids}")
        assert scene_drone.name in actor_ids
    except NNGException as err:
        raise Exception(str(err))


def test_list_assets(client, scene_world):
    try:
        asset_ids = scene_world.list_assets(".*")
        print(f"Received actor_ids: {asset_ids}")
        # Assets required by other tests
        assert "BasicLandingPad" in asset_ids
        assert "OrangeBall_Blueprint" in asset_ids
    except NNGException as err:
        raise Exception(str(err))


def test_list_objects(client, scene_world):
    try:
        name_regex = ".*"
        scene_objects_list = scene_world.list_objects(name_regex)
        print("object_list:", scene_objects_list)
        assert len(scene_objects_list)

    except NNGException as err:
        raise Exception(str(err))


def test_get_object_pose(client, scene_world):
    try:
        object_id: str = "OrangeBall"  # Present in Blocks env
        object_pose = scene_world.get_object_pose(object_id)
        print("object_pose:", object_pose)
        assert object_pose is not None
        assert isinstance(object_pose, Pose)
        assert object_pose.translation
        assert isinstance(object_pose.translation, Vector3)
        assert object_pose.rotation
        assert isinstance(object_pose.rotation, Quaternion)
        assert not any([math.isnan(x) for x in object_pose.translation.to_list()])

    except NNGException as err:
        raise Exception(str(err))


def test_get_object_poses(client, scene_world):
    try:
        # "OrangeBall" is a scene object in Blocks env, "Drone1" is the spawned robot
        # that can also be queried as an object
        object_ids = ["OrangeBall", "Drone1"]
        object_poses = scene_world.get_object_poses(object_ids)
        print("object_poses:", object_poses)
        assert object_poses is not None
        assert len(object_poses) == 2

        pose1 = object_poses[0]
        assert pose1.translation
        assert isinstance(pose1.translation, Vector3)
        assert pose1.rotation
        assert isinstance(pose1.rotation, Quaternion)
        assert not any([math.isnan(x) for x in pose1.translation.to_list()])

        pose2 = object_poses[1]
        assert pose2.translation
        assert isinstance(pose2.translation, Vector3)
        assert pose2.rotation
        assert isinstance(pose2.rotation, Quaternion)
        assert not any([math.isnan(x) for x in pose2.translation.to_list()])

    except NNGException as err:
        raise Exception(str(err))


def test_get_object_scale(client, scene_world):
    try:
        object_id: str = "OrangeBall"  # Present in Blocks env
        object_scale = scene_world.get_object_scale(object_id)
        print("object_scale:", object_scale)
        assert object_scale is not None
        assert not any(
            [math.isnan(x) for x in object_scale]
        )  # Scale returns zeros if object doesn't exist

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_destroy_object(client, scene_world):
    try:
        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TempLandingPad"
        asset_name: str = "BasicLandingPad"  # Existing asset
        pose = _make_pose(20, 1, -5)
        scale = [10.0, 10.0, 10.0]
        enable_physics: bool = False
        retval = scene_world.spawn_object(
            object_name, asset_name, pose, scale, enable_physics
        )

        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = scene_world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = scene_world.destroy_object(object_name)
        assert status
        objects = scene_world.list_objects(object_name + ".*")
        assert not len(objects)

        # ------------------------------------------------------------------
        # Check spawning a Blueprint object with its physics disabled
        object_name: str = "TempBPOrangeBall"
        asset_name: str = "OrangeBall_Blueprint"  # Existing asset
        pose = _make_pose(20, 1, -5)
        scale = [1.0, 1.0, 1.0]
        enable_physics: bool = False
        retval = scene_world.spawn_object(
            object_name, asset_name, pose, scale, enable_physics
        )

        print("Spawned Blueprint object successfully with name: ", retval)
        new_objs = scene_world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = scene_world.destroy_object(object_name)
        assert status
        objects = scene_world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_object_at_geo(client, scene_world):
    try:
        # Loaded scene_test_drone.jsonc has the following home geo point to compare
        # with spawned object lat/lon/alt below:
        #   "home-geo-point": {
        #       "latitude": 47.641468,
        #       "longitude": -122.140165,
        #       "altitude": 122.0
        #   },

        # Check spawning object with its physics disabled
        object_name: str = "TestLandingPadGeo"
        asset_name: str = "BasicLandingPad"  # Existing asset
        lat = 47.641468
        lon = -122.140165
        alt = 125.0  # 3 m above scene's home geo point above
        rotation = [1, 0, 0, 0]
        # Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        scale = [1.0, 1.0, 1.0]
        enable_physics: bool = False
        retval = scene_world.spawn_object_at_geo(
            object_name, asset_name, lat, lon, alt, rotation, scale, enable_physics
        )

        print("Spawned object successfully with name: ", retval)
        new_objs = scene_world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        object_pose = scene_world.get_object_pose(object_name)
        print("object_pose:", object_pose)
        assert object_pose.translation

        # Expect position to be 3 m above NED origin
        expected_translation = Vector3({"x": 0, "y": 0, "z": -3})
        zipped_translation = zip(
            object_pose.translation.to_list(), expected_translation.to_list()
        )
        for res, exp in zipped_translation:
            # Tolerance is needed because of inaccuracies in ECEF-NED conversion
            assert math.isclose(res, exp, abs_tol=0.02)

        # Check destroying object that was just spawned
        status = scene_world.destroy_object(object_name)
        assert status
        objects = scene_world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_object_from_file(client, scene_world):
    try:
        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TempLandingPadFile"
        asset_name: str = "BasicLandingPad"  # Existing asset
        pose = _make_pose(20, 1, -5)
        scale = [10.0, 10.0, 10.0]
        enable_physics: bool = False
        binary_gltf: bool = True
        file1 = open("assets/BasicLandingPad.glb", "rb")
        gltf_byte_array = file1.read()
        file1.close()
        retval = scene_world.spawn_object_from_file(
            object_name,
            "gltf",
            gltf_byte_array,
            binary_gltf,
            pose,
            scale,
            enable_physics,
        )
        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = scene_world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = scene_world.destroy_object(object_name)
        assert status
        objects = scene_world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_object_from_file_at_geo(client, scene_world):
    try:
        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TempLandingPadFileGeo"
        asset_name: str = "BasicLandingPad"  # Existing asset

        lat = 47.641468
        lon = -122.140165
        alt = 125.0  # 3 m above scene's home geo point above
        rotation = [1, 0, 0, 0]
        scale = [10.0, 10.0, 10.0]
        enable_physics: bool = False
        binary_gltf: bool = True
        file1 = open("assets/BasicLandingPad.glb", "rb")
        gltf_byte_array = file1.read()
        file1.close()
        retval = scene_world.spawn_object_from_file_at_geo(
            object_name,
            "gltf",
            gltf_byte_array,
            binary_gltf,
            lat,
            lon,
            alt,
            rotation,
            scale,
            enable_physics,
        )
        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = scene_world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = scene_world.destroy_object(object_name)
        assert status
        objects = scene_world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_destroy_all_spawned_objects(client, scene_world):
    try:
        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TempLandingPadBulk"
        asset_name: str = "BasicLandingPad"  # Existing asset
        for i in range(10):
            pose = _make_pose(20 + i, 1, -5)
            scale = [10.0, 10.0, 10.0]
            enable_physics: bool = False
            retval = scene_world.spawn_object(
                object_name, asset_name, pose, scale, enable_physics
            )

        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = scene_world.list_objects(object_name + ".*")
        assert len(new_objs) >= 10

        # Check destroying object that was just spawned
        status = scene_world.destroy_all_spawned_objects()
        assert status
        objects = scene_world.list_objects(object_name + ".*")
        assert not len(objects)
    except NNGException as err:
        raise Exception(str(err))


def test_enable_disable_weather_visual_effects(client, scene_world):
    try:
        status = scene_world.enable_weather_visual_effects()
        assert status is True

        status = scene_world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_weather_visual_effects_param(client, scene_world):
    try:
        status = scene_world.enable_weather_visual_effects()
        assert status is True
        status = scene_world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.9)
        assert status is True
        status = scene_world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_reset_weather_visual_effects(client, scene_world):
    try:
        status = scene_world.enable_weather_visual_effects()
        assert status is True
        status = scene_world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.9)
        assert status is True
        status = scene_world.reset_weather_effects()
        assert status is True
        status = scene_world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_get_weather_visual_effects(client, scene_world):
    try:
        status = scene_world.enable_weather_visual_effects()
        assert status is True
        status = scene_world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.9)
        assert status is True
        weather_dict = scene_world.get_weather_visual_effects_param()
        assert weather_dict[WeatherParameter.RAIN] == pytest.approx(0.9)
        status = scene_world.set_weather_visual_effects_param(WeatherParameter.SNOW, 0.4)
        assert status is True
        weather_dict = scene_world.get_weather_visual_effects_param()
        assert weather_dict[WeatherParameter.RAIN] == pytest.approx(0.9)
        assert weather_dict[WeatherParameter.SNOW] == pytest.approx(0.4)
        status = scene_world.reset_weather_effects()
        assert status is True
        weather_dict = scene_world.get_weather_visual_effects_param()
        assert weather_dict[WeatherParameter.RAIN] == pytest.approx(0)
        assert weather_dict[WeatherParameter.SNOW] == pytest.approx(0)
        status = scene_world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_wind_velocity(client, scene_world):
    try:
        scene_world.set_wind_velocity(5.0, 5.0, 5.0)

        assert scene_world.get_wind_velocity() == pytest.approx((5.0, 5.0, 5.0))

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_material(client, scene_world, scene_drone):
    try:
        object_name = "TempMaterialBall"
        asset_name = "OrangeBall_Blueprint"
        pose = _make_pose(20, 0, -5)
        scale = [1.0, 1.0, 1.0]

        scene_world.spawn_object(object_name, asset_name, pose, scale, False)

        mean0 = _capture_object_mean(scene_drone, scene_world, object_name)

        material_path = "/ProjectAirSim/Weather/WeatherFX/Materials/M_Leaf_master"
        status = scene_world.set_object_material(object_name, material_path)
        assert status is True

        time.sleep(1.0)
        mean1 = _capture_object_mean(scene_drone, scene_world, object_name)

        assert float(np.linalg.norm(mean1 - mean0)) >= 0.05
        scene_world.destroy_object(object_name)

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_texture_from_file(client, scene_world, scene_drone):
    try:
        object_name = "TempTextureBall"
        asset_name = "OrangeBall_Blueprint"
        texture_path = "assets/sample_texture.png"
        pose = _make_pose(20, 0, -5)
        scale = [1.0, 1.0, 1.0]

        scene_world.spawn_object(object_name, asset_name, pose, scale, False)

        mean0 = _capture_object_mean(scene_drone, scene_world, object_name)

        status = scene_world.set_object_texture_from_file(object_name, texture_path)
        assert status is True

        time.sleep(1.0)
        mean1 = _capture_object_mean(scene_drone, scene_world, object_name)

        assert float(np.linalg.norm(mean1 - mean0)) >= 0.05
        scene_world.destroy_object(object_name)

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_texture_from_packaged_asset(client, scene_world, scene_drone):
    try:
        object_name = "TempTextureCone"
        asset_name = "1M_Cube"
        texture_path = "/Game/Geometry/Textures/T_Default_Material_Grid_M"
        pose = _make_pose(20, 0, -5)
        scale = [3.0, 3.0, 3.0]

        scene_world.spawn_object(object_name, asset_name, pose, scale, False)

        mean0 = _capture_object_mean(scene_drone, scene_world, object_name)

        status = scene_world.set_object_texture_from_packaged_asset(object_name, texture_path)
        assert status is True

        time.sleep(1.0)
        mean1 = _capture_object_mean(scene_drone, scene_world, object_name)

        assert float(np.linalg.norm(mean1 - mean0)) >= 0.03
        scene_world.destroy_object(object_name)

    except NNGException as err:
        raise Exception(str(err))


def test_swap_object_texture(client, scene_world, scene_drone):
    try:
        object_actor_tag = "ball"
        object_name = "TempSwapBall"
        asset_name = "OrangeBall_Blueprint"
        pose = _make_pose(20, 0, -5)
        scale = [1.0, 1.0, 1.0]

        scene_world.spawn_object(object_name, asset_name, pose, scale, False)

        mean0 = _capture_object_mean(scene_drone, scene_world, object_name)

        swapped_objects = scene_world.swap_object_texture(object_actor_tag, 1)
        assert len(swapped_objects) > 0

        time.sleep(1.0)
        mean1 = _capture_object_mean(scene_drone, scene_world, object_name)

        swapped_objects = scene_world.swap_object_texture(object_actor_tag, 0)
        assert len(swapped_objects) > 0

        time.sleep(1.0)
        mean2 = _capture_object_mean(scene_drone, scene_world, object_name)

        assert float(np.linalg.norm(mean1 - mean0)) >= 0.03 or float(
            np.linalg.norm(mean2 - mean1)
        ) >= 0.03
        scene_world.destroy_object(object_name)

    except NNGException as err:
        raise Exception(str(err))
    
def test_set_light_object_intensity(client, scene_world):
    try:
        spotlight_asset_path = "SpotLightActor"
        scale = [1.0, 1.0, 1.0]
        enable_physics = False
        spot_light_object_name = "TempSpotLightIntensity"
        pose = _make_pose(0.0, 8.0, -5.0)
        scene_world.spawn_object(
            spot_light_object_name, spotlight_asset_path, pose, scale, enable_physics
        )

        pose_before = scene_world.get_object_pose(spot_light_object_name)
        status = scene_world.set_light_object_intensity(spot_light_object_name, 10000.0)
        assert status is True
        pose_after = scene_world.get_object_pose(spot_light_object_name)
        assert pose_after.translation.to_list() == pytest.approx(
            pose_before.translation.to_list(), abs=1e-3
        )
        scene_world.destroy_object(spot_light_object_name)

    except NNGException as err:
        raise Exception(str(err))
    
def test_set_light_object_color(client, scene_world):
    try:
        spotlight_asset_path = "SpotLightActor"
        scale = [1.0, 1.0, 1.0]
        enable_physics = False
        spot_light_object_name = "TempSpotLightColor"
        pose = _make_pose(0.0, 8.0, -5.0)
        scene_world.spawn_object(
            spot_light_object_name, spotlight_asset_path, pose, scale, enable_physics
        )
        pose_before = scene_world.get_object_pose(spot_light_object_name)
        color_rgb = [1.0, 0.0, 0.0]
        status = scene_world.set_light_object_color(spot_light_object_name, color_rgb)
        assert status is True
        pose_after = scene_world.get_object_pose(spot_light_object_name)
        assert pose_after.translation.to_list() == pytest.approx(
            pose_before.translation.to_list(), abs=1e-3
        )
        scene_world.destroy_object(spot_light_object_name)

    except NNGException as err:
        raise Exception(str(err))

def test_set_light_object_radius(client, scene_world):
    try:
        spot_light_asset_path = "SpotLightActor"
        scale = [1.0, 1.0, 1.0]
        enable_physics = False
        spot_light_object_name = "TempSpotLightRadius"
        pose = _make_pose(0.0, 8.0, -5.0)
        scene_world.spawn_object(
            spot_light_object_name, spot_light_asset_path, pose, scale, enable_physics
        )

        pose_before = scene_world.get_object_pose(spot_light_object_name)
        status = scene_world.set_light_object_radius(spot_light_object_name, 16000.0)
        assert status is True
        pose_after = scene_world.get_object_pose(spot_light_object_name)
        assert pose_after.translation.to_list() == pytest.approx(
            pose_before.translation.to_list(), abs=1e-3
        )

        directional_light_asset_path = "DirectionalLightActor"
        directional_light_object_name = scene_world.spawn_object(
            "TempDirLightRadius", directional_light_asset_path, pose, scale, enable_physics
        )

        status = scene_world.set_light_object_radius(directional_light_object_name, 16000.0)
        assert status is False
        scene_world.destroy_object(spot_light_object_name)
        scene_world.destroy_object(directional_light_object_name)

    except NNGException as err:
        raise Exception(str(err))

def test_set_time_of_day(client, scene_world):
    try:
        enabled = True
        now_datetime = "2020-01-01 12:00:00"
        move_sun = True
        is_start_datetime_dst = True

        status = scene_world.set_time_of_day(
            enabled, now_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_get_time_of_day(client, scene_world):
    try:
        enabled = True
        now_datetime = "2020-01-01 12:00:00"
        move_sun = True
        is_start_datetime_dst = True

        status = scene_world.set_time_of_day(
            enabled, now_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )
        assert status is True
        time_str = scene_world.get_time_of_day()
        assert time_str == now_datetime

        now_datetime = "2020-01-01 09:00:00"
        status = scene_world.set_time_of_day(
            enabled, now_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )
        assert status is True
        time_str = scene_world.get_time_of_day()
        assert time_str == now_datetime

    except NNGException as err:
        raise Exception(str(err))


def test_set_sun_position_from_datetime(client, scene_world):
    try:
        # Stop the time of day feature
        status = False
        cmd_datetime = "2020-01-01 21:00:00"
        move_sun = False
        is_start_datetime_dst = True

        status = scene_world.set_time_of_day(
            status, cmd_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )

        # Try setting to current sun position
        now = datetime.now()

        status = scene_world.set_sun_position_from_date_time(now, False)
        assert status is True

        # Set back to daytime
        daytime = datetime(now.year, now.month, now.day, 12, 0, 0)

        status = scene_world.set_sun_position_from_date_time(daytime, False)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_and_get_sun_intensity(client, scene_world):
    try:
        val = 2.5
        status = scene_world.set_sunlight_intensity(val)
        assert status is True
        get_val = scene_world.get_sunlight_intensity()
        assert get_val == val

    except NNGException as err:
        raise Exception(str(err))


def test_set_and_get_cloud_shadow_strength(client, scene_world):
    try:
        val = 0.5
        status = scene_world.set_cloud_shadow_strength(val)
        assert status is True
        get_val = scene_world.get_cloud_shadow_strength()
        assert get_val == val

    except NNGException as err:
        raise Exception(str(err))


def test_failure_response_handling(client, scene_world):
    try:
        bad_sim_get_obj_pose_req: Dict = {
            "method": f"{scene_world.parent_topic}/get_object_pose",
            # Intentionally leave out the required `object_name` parameter
            "params": {},
            "version": 1.0,
        }

        # Check that a bad request raises a client-side RuntimeError exception
        with pytest.raises(RuntimeError) as exc_info:
            client.request(bad_sim_get_obj_pose_req)

        print(f'\n  Response of bad request: "{exc_info.value}"')
    except NNGException as err:
        raise Exception(str(err))


def test_get_set_segmentation_id(client, scene_world):
    try:
        seg_id_orig = scene_world.get_segmentation_id_by_name("templatecube_rounded_1", True)
        assert seg_id_orig != -1

        seg_id_new = (seg_id_orig + 1) % 256
        status = scene_world.set_segmentation_id_by_name(
            "templatecube_rounded.*", seg_id_new, True, True
        )
        assert status is True

        seg_id_resp = scene_world.get_segmentation_id_by_name("templatecube_rounded_1", True)
        assert seg_id_resp == seg_id_new
        # TODO Add tests for each variation of parameters

        seg_id_map = scene_world.get_segmentation_id_map()
        assert seg_id_map["TemplateCube_Rounded_1"] == seg_id_new

    except NNGException as err:
        raise Exception(str(err))


def test_switch_streaming_view(client, scene_world, reset_scene_drone):
    try:
        drone = reset_scene_drone

        img0 = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
        size0 = (int(img0["width"]), int(img0["height"]))
        assert size0[0] > 0 and size0[1] > 0

        assert scene_world.switch_streaming_view() is True
        img1 = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
        assert (int(img1["width"]), int(img1["height"])) == size0

        assert scene_world.switch_streaming_view() is True
        img2 = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
        assert (int(img2["width"]), int(img2["height"])) == size0

    except NNGException as err:
        raise Exception(str(err))


def test_plot_debug_markers(client, scene_world):
    try:
        list = [1, 2, 3, 4, 5, 6]
        # ----------------------------------------------------------------------------------
        points = [[x, y, -5] for x, y in zip(list, list)]
        color_rgba = [1.0, 0.0, 0.0, 1.0]
        size = 10
        duration = 10
        is_persistent = True
        status = scene_world.plot_debug_points(
            points, color_rgba, size, duration, is_persistent
        )
        assert status is True

        points_start = [[x, y, z] for x, y, z in zip(list, list, list)]
        points_end = [[x, y, z] for x, y, z in zip(list, list, list)]
        color_rgba = [1.0, 0.0, 1.0, 1.0]
        thickness = 3
        size = 15
        is_persistent = False
        status = scene_world.plot_debug_arrows(
            points_start,
            points_end,
            color_rgba,
            thickness,
            size,
            duration,
            is_persistent,
        )
        assert status is True

        points = [[x, y, -5] for x, y in zip(list, list)]
        color_rgba = [1.0, 0.0, 0.0, 1.0]
        thickness = 5
        status = scene_world.plot_debug_solid_line(
            points, color_rgba, thickness, duration, is_persistent
        )
        assert status is True

        points = [[x, y, -7] for x, y in zip(list, list)]
        color_rgba = [0.0, 1.0, 0.0, 1.0]
        status = scene_world.plot_debug_dashed_line(
            points, color_rgba, thickness, duration, is_persistent
        )
        assert status is True

        positions = [[x, y, -1] for x, y in zip(list, list)]
        strings = ["Microsoft AirSim" for i in range(len(positions))]
        scale = 1
        color_rgba = [1.0, 1.0, 1.0, 1.0]
        status = scene_world.plot_debug_strings(
            strings, positions, scale, color_rgba, duration
        )
        assert status is True

        translations = [[x, y, -3] for x, y in zip(list, list)]
        rotations = [[r, r, r, r] for r in list]
        poses = []

        for trans, rot in zip(translations, rotations):
            trans = Vector3({"x": trans[0], "y": trans[1], "z": trans[2]})
            rot = Quaternion({"w": rot[0], "x": rot[1], "y": rot[2], "z": rot[3]})
            poses.append(
                Pose(
                    {
                        "translation": trans,
                        "rotation": rot,
                        "frame_id": "DEFAULT_ID",
                    }
                )
            )
        scale = 35
        status = scene_world.plot_debug_transforms(
            poses, scale, thickness, duration, is_persistent
        )
        assert status is True

        names = ["yaw = " + str(round(yaw, 1)) for yaw in list]
        text_scale = 1
        status = scene_world.plot_debug_transforms_with_names(
            poses, names, scale, thickness, text_scale, color_rgba, duration
        )
        assert status is True

        status = scene_world.flush_persistent_markers()
        assert status is True
        status = scene_world.flush_persistent_markers()
        assert status is True

        status = scene_world.plot_debug_points(
            points, color_rgba, size, duration, is_persistent=False
        )
        assert status is True
        status = scene_world.flush_persistent_markers()
        assert status is True
    except NNGException as err:
        raise Exception(str(err))


def test_trace_line(client, scene_world):
    try:
        assert scene_world.toggle_trace() is True
        assert scene_world.toggle_trace() is True

        assert scene_world.set_trace_line([0.0, 0.0, 1.0, 1.0], 5.0) is True

    except NNGException as err:
        raise Exception(str(err))


def test_segmentation_id_to_color(client):
    try:
        seg_id = 1
        expected_color = Color([153, 108, 6])
        result_color = segmentation_id_to_color(seg_id)
        assert result_color == expected_color

        result_seg_id = segmentation_color_to_id(result_color)
        assert result_seg_id == seg_id

    except NNGException as err:
        raise Exception(str(err))


def test_get_kinematics(client, scene_drone):
    try:
        kin = scene_drone.get_ground_truth_kinematics()
        pose = scene_drone.get_ground_truth_pose()

        assert "time_stamp" in kin

        assert "pose" in kin
        assert "position" in kin["pose"]
        assert "orientation" in kin["pose"]

        for axis in ("x", "y", "z"):
            assert kin["pose"]["position"][axis] == pytest.approx(
                pose["translation"][axis], abs=1e-3
            )
        qk, qp = kin["pose"]["orientation"], pose["rotation"]
        for axis in ("w", "x", "y", "z"):
            assert qk[axis] == pytest.approx(qp[axis], abs=1e-3)
        q = kin["pose"]["orientation"]
        qn = math.sqrt(q["w"] ** 2 + q["x"] ** 2 + q["y"] ** 2 + q["z"] ** 2)
        assert qn == pytest.approx(1.0, abs=1e-2)

        assert "twist" in kin
        assert "linear" in kin["twist"]
        assert "angular" in kin["twist"]

        assert "accels" in kin
        assert "linear" in kin["accels"]
        assert "angular" in kin["accels"]

        lin = kin["twist"]["linear"]
        ang = kin["twist"]["angular"]
        for axis in ("x", "y", "z"):
            assert abs(lin[axis]) < 50.0
            assert abs(ang[axis]) < 50.0
    except NNGException as err:
        raise Exception(str(err))


def test_set_kinematics(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        kin = drone.get_ground_truth_kinematics()
        kin["pose"]["position"]["x"] = 123.4
        assert drone.set_ground_truth_kinematics(kin) is True
        kin = drone.get_ground_truth_kinematics()
        assert kin["pose"]["position"]["x"] == pytest.approx(123.4, 0.1)

    except NNGException as err:
        raise Exception(str(err))


def test_get_ground_truth_geo_location(client, scene_drone):
    try:
        loc = scene_drone.get_ground_truth_geo_location()

        assert "latitude" in loc
        assert "longitude" in loc
        assert "altitude" in loc
        home = scene_drone.home_geo_point
        assert loc["latitude"] == pytest.approx(home["latitude"], abs=0.002)
        assert loc["longitude"] == pytest.approx(home["longitude"], abs=0.002)
        assert loc["altitude"] == pytest.approx(home["altitude"] + 4.0, abs=2.0)
    except NNGException as err:
        raise Exception(str(err))


def test_can_arm(client, scene_drone):
    try:
        can_arm = scene_drone.can_arm()
        assert can_arm is True

    except NNGException as err:
        raise Exception(str(err))


def test_get_estimated_geo_location(client, scene_drone):
    try:
        loc = scene_drone.get_estimated_geo_location()
        loc_gt = scene_drone.get_ground_truth_geo_location()

        assert "latitude" in loc
        assert "longitude" in loc
        assert "altitude" in loc
        assert loc["latitude"] == pytest.approx(loc_gt["latitude"], abs=0.01)
        assert loc["longitude"] == pytest.approx(loc_gt["longitude"], abs=0.01)
        assert loc["altitude"] == pytest.approx(loc_gt["altitude"], abs=5.0)
    except NNGException as err:
        raise Exception(str(err))


def test_get_ready_state(client, scene_drone):
    try:
        state = scene_drone.get_ready_state()

        assert "ready_val" in state
        assert "ready_message" in state
        assert isinstance(state["ready_val"], (int, float))
        assert isinstance(state["ready_message"], str)
    except NNGException as err:
        raise Exception(str(err))


def test_get_estimated_kinematics(client, scene_drone):
    try:
        kin = scene_drone.get_estimated_kinematics()
        kin_gt = scene_drone.get_ground_truth_kinematics()

        assert "time_stamp" in kin

        assert "pose" in kin
        assert "position" in kin["pose"]
        assert "orientation" in kin["pose"]

        for axis in ("x", "y", "z"):
            assert kin["pose"]["position"][axis] == pytest.approx(
                kin_gt["pose"]["position"][axis], abs=5.0
            )
        q = kin["pose"]["orientation"]
        qn = math.sqrt(q["w"] ** 2 + q["x"] ** 2 + q["y"] ** 2 + q["z"] ** 2)
        assert qn == pytest.approx(1.0, abs=1e-2)

        assert "twist" in kin
        assert "linear" in kin["twist"]
        assert "angular" in kin["twist"]

        assert "accels" in kin
        assert "linear" in kin["accels"]
        assert "angular" in kin["accels"]
    except NNGException as err:
        raise Exception(str(err))


def test_landed_state(client, scene_drone):
    try:
        landed_state = scene_drone.get_landed_state()
        assert landed_state == LandedState.LANDED

    except NNGException as err:
        raise Exception(str(err))


def test_battery_state(client):
    try:
        world = World(client, "scene_battery_simple.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        assert drone.set_battery_remaining(0.99) is True

        data = drone.get_battery_state("Battery")

        assert "time_stamp" in data

        assert "battery_pct_remaining" in data
        assert "estimated_time_remaining" in data
        assert "battery_charge_state" in data
        assert data["battery_pct_remaining"] == pytest.approx(0.99, 0.1)
    except NNGException as err:
        raise Exception(str(err))


def test_battery_drain_rate(client):
    try:
        world = World(client, "scene_battery_simple.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        assert drone.set_battery_drain_rate(0.01) is True

        data = drone.get_battery_drain_rate("Battery")
        assert data == pytest.approx(0.01, 0.001)

    except NNGException as err:
        raise Exception(str(err))


def test_battery_health_status(client):
    try:
        world = World(client, "scene_battery_simple.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        assert drone.set_battery_health_status(False) is True

        data = drone.get_battery_state("Battery")
        assert data["battery_charge_state"] == "BATTERY_CHARGE_STATE_UNHEALTHY"

    except NNGException as err:
        raise Exception(str(err))


def test_get_set_pose(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        trans = Vector3({"x": 1.0, "y": 2.0, "z": 3.0})
        rot = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
        pose = Pose(
            {
                "translation": trans,
                "rotation": rot,
                "frame_id": "DEFAULT_ID",
            }
        )
        drone.set_pose(pose)

        pose = drone.get_ground_truth_pose()

        assert "translation" in pose

        assert "x" in pose["translation"]
        assert pose["translation"]["x"] == pytest.approx(1.0)
        assert "y" in pose["translation"]
        assert pose["translation"]["y"] == pytest.approx(2.0)
        assert "z" in pose["translation"]
        assert pose["translation"]["z"] == pytest.approx(3.0)

    except NNGException as err:
        raise Exception(str(err))


def test_get_set_geo_pose(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        # Loaded scene_test_drone.jsonc has the following home geo point to compare
        # with spawned object lat/lon/alt below:
        home_geo_point = {
            "latitude": 47.641468,
            "longitude": -122.140165,
            "altitude": 122.0,
        }

        lat = 47.641460
        lon = -122.140160
        alt = 125.0

        expected = geo_to_ned_coordinates(home_geo_point, [lat, lon, alt])

        rot = Quaternion({"w": 1.0, "x": 0, "y": 0, "z": 0})

        drone.set_geo_pose(lat, lon, alt, rot)

        pose = drone.get_ground_truth_pose()

        assert "translation" in pose

        assert "x" in pose["translation"]
        assert pose["translation"]["x"] == pytest.approx(expected[0])
        assert "y" in pose["translation"]
        assert pose["translation"]["y"] == pytest.approx(expected[1])
        assert "z" in pose["translation"]
        assert pose["translation"]["z"] == pytest.approx(expected[2])

    except NNGException as err:
        raise Exception(str(err))


def test_get_images(client, scene_world, scene_drone):
    try:
        # Test getting a single image type
        cur_simtime = scene_world.get_sim_time()
        images = scene_drone.get_images(
            camera_id="DownCamera", image_type_ids=[ImageType.SCENE]
        )

        # Check that image was captured after request was submitted
        print(f"Get scene image requested at simtime={cur_simtime}")
        print(
            f'Scene image received for simtime={images[ImageType.SCENE]["time_stamp"]}'
        )
        assert images[ImageType.SCENE]["time_stamp"] >= cur_simtime

        # Check that image is not empty
        assert len(images[ImageType.SCENE]["data"]) > 0

        # Test getting a set of image types
        cur_simtime = scene_world.get_sim_time()
        images = scene_drone.get_images(
            camera_id="DownCamera",
            image_type_ids=[ImageType.SCENE, ImageType.DEPTH_PERSPECTIVE],
        )

        # Check that images were captured after request was submitted
        print(f"Get scene and depth images requested at simtime={cur_simtime}")
        print(
            f'Scene image received for simtime={images[ImageType.SCENE]["time_stamp"]}'
        )
        print(
            f"Depth image received for simtime="
            f'{images[ImageType.DEPTH_PERSPECTIVE]["time_stamp"]}'
        )
        assert images[ImageType.SCENE]["time_stamp"] >= cur_simtime
        assert images[ImageType.DEPTH_PERSPECTIVE]["time_stamp"] >= cur_simtime
        assert (
            images[ImageType.SCENE]["time_stamp"]
            == images[ImageType.DEPTH_PERSPECTIVE]["time_stamp"]
        )

        # Check that images are not empty
        assert len(images[ImageType.SCENE]["data"]) > 0
        assert len(images[ImageType.DEPTH_PERSPECTIVE]["data"]) > 0

        # Test getting an empty set of image types
        images = scene_drone.get_images(camera_id="DownCamera", image_type_ids=[])

        # Check that an empty dict is returned
        assert not images

    except NNGException as err:
        raise Exception(str(err))


def test_set_camera_pose(client, reset_scene_drone):
    drone = reset_scene_drone

    z0 = INITIAL_SCENE_DRONE_POSE["translation"]["z"]
    trans = Vector3({"x": 50, "y": 60, "z": 70})
    rot = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
    pose = Pose(
        {
            "translation": trans,
            "rotation": rot,
            "frame_id": "DEFAULT_ID",
        }
    )

    assert drone.set_camera_pose("DownCamera", pose) is True
    img = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
    assert img["pos_x"] == pytest.approx(50.0, abs=3.0)
    assert img["pos_y"] == pytest.approx(60.0, abs=3.0)
    assert img["pos_z"] == pytest.approx(70.0, abs=abs(z0))


def test_set_camera_focal_length(client, scene_world, reset_scene_drone):
    drone = reset_scene_drone

    scene_world.pause()
    before = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
    std0 = float(np.std(unpack_image(before)))
    scene_world.resume()

    assert drone.set_focal_length("DownCamera", ImageType.SCENE, 15.0) is True

    scene_world.pause()
    after = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
    std1 = float(np.std(unpack_image(after)))
    scene_world.resume()

    assert before["width"] == after["width"]
    assert before["height"] == after["height"]
    assert abs(std1 - std0) > 1e-6 or std1 > 0.0


def test_set_field_of_view(client, scene_world, reset_scene_drone):
    drone = reset_scene_drone

    scene_world.pause()
    before = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
    std0 = float(np.std(unpack_image(before)))
    scene_world.resume()

    assert drone.set_field_of_view("DownCamera", ImageType.SCENE, 1.0) is True

    scene_world.pause()
    after = drone.get_images("DownCamera", [ImageType.SCENE])[ImageType.SCENE]
    std1 = float(np.std(unpack_image(after)))
    scene_world.resume()

    assert before["width"] == after["width"]
    assert before["height"] == after["height"]
    assert abs(std1 - std0) > 1e-6 or std1 > 0.0


def test_create_voxel_grid(client, scene_world):
    try:
        center = (0, 0, 0)
        center_trans = Vector3({"x": center[0], "y": center[1], "z": center[2]})
        center_rot = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
        center_pos = Pose(
            {
                "translation": center_trans,
                "rotation": center_rot,
                "frame_id": "DEFAULT_ID",
            }
        )
        x_size, y_size, z_size = 50, 50, 50
        resolution = 1

        occupancy_grid = scene_world.create_voxel_grid(
            center_pos, x_size, y_size, z_size, resolution
        )

        check = all(element == occupancy_grid[0] for element in occupancy_grid)

        assert len(occupancy_grid) != 0
        assert check == False  # Make sure the entire array is not either True or False

    except NNGException as err:
        raise Exception(str(err))


def test_get_bbox_3d(client, scene_world):
    try:
        object_id = "Cone_5"
        bbox_data = scene_world.get_3d_bounding_box(object_id, BoxAlignment.WORLD_AXIS)
        print(f"bbox_data:{bbox_data}")
        expected_fields = [
            "center",
            "quaternion",
            "size",
        ]
        actual_fields = bbox_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)
        assert sorted(["x", "y", "z"]) == sorted(bbox_data["center"].keys())
        assert sorted(["x", "y", "z"]) == sorted(bbox_data["size"].keys())
        assert sorted(["x", "y", "z", "w"]) == sorted(bbox_data["quaternion"].keys())

    except NNGException as err:
        raise Exception(str(err))


def test_get_unexisting_bbox_3d(client, scene_world):
    try:
        object_id = "UnexistingObject"
        bbox_data = scene_world.get_3d_bounding_box(object_id, BoxAlignment.WORLD_AXIS)
        print(f"bbox_data:{bbox_data}")
        assert bbox_data == {}

    except NNGException as err:
        raise Exception(str(err))


def test_get_bbox_3d_spawned(client, scene_world):
    try:
        object_name: str = "TestCube"
        asset_name: str = "1M_Cube"  # Existing asset
        ref_frame = "DEFAULT_ID"
        # Spawn above ground to not fall out of bounds
        translation = Vector3({"x": 10, "y": 10, "z": -10})
        rotation = Quaternion({"w": 1, "x": 0.5, "y": 0, "z": 0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        scale = [10.0, 10.0, 20.0]
        enable_physics: bool = False
        scene_world.spawn_object(object_name, asset_name, pose, scale, enable_physics)

        # Get world-aligned bbox
        bbox_world = scene_world.get_3d_bounding_box(object_name, BoxAlignment.WORLD_AXIS)
        print(bbox_world)
        assert Vector3(bbox_world["center"]) == translation
        assert bbox_world["quaternion"] == {"w": 1, "x": 0.0, "y": 0, "z": 0}

        # Get object-aligned bbox
        bbox_obj = scene_world.get_3d_bounding_box(object_name, BoxAlignment.OBJECT_ORIENTED)
        print(bbox_obj)
        assert Vector3(bbox_obj["center"]) == translation
        assert bbox_obj["quaternion"] != bbox_world["quaternion"]
        size_list = Vector3(bbox_obj["size"]).to_list()
        for i in range(0, 3):
            assert abs(size_list[i] - scale[i]) < 1e-4

        # cleanup
        scene_world.destroy_object(object_name)

    except NNGException as err:
        raise Exception(str(err))


def test_manual_controller(client):
    try:
        world = World(client, "scene_test_manual_controller_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        # Test manually setting a single control value
        result_single = drone.set_control_signals({"Prop_FL_actuator": 0.00001})
        assert result_single is True

        # Test manually setting a single control value for an invalid actuator
        result_invalid_actuator = drone.set_control_signals(
            {"Invalid_actuator": 0.00001}
        )
        assert result_invalid_actuator is False

        # Test manually setting multiple control values
        result_multiple = drone.set_control_signals(
            {
                "Prop_FR_actuator": 0.00002,
                "Prop_RL_actuator": 0.00003,
                "Prop_RR_actuator": 0.00004,
            }
        )
        assert result_multiple is True

        # Test manually setting multiple control values that include an invalid actuator
        result_partial = drone.set_control_signals(
            {
                "Prop_FR_actuator": 0.00002,
                "Invalid_actuator": 0.00003,
                "Prop_RR_actuator": 0.00004,
            }
        )
        assert result_partial is False

    except NNGException as err:
        raise Exception(str(err))


async def request_control_async(drone):
    request_control = await drone.request_control_async()
    await request_control


def test_request_control_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        assert drone.enable_api_control() is True
        asyncio.run(request_control_async(drone))
        assert drone.is_api_control_enabled() is True

    except NNGException as err:
        raise Exception(str(err))


async def set_mission_mode_async(drone):
    set_mode = await drone.set_mission_mode_async()
    await set_mode


def test_set_mission_mode_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        assert drone.enable_api_control() is True
        asyncio.run(set_mission_mode_async(drone))
        kin = drone.get_ground_truth_kinematics()
        q = kin["pose"]["orientation"]
        qn = math.sqrt(q["w"] ** 2 + q["x"] ** 2 + q["y"] ** 2 + q["z"] ** 2)
        assert qn == pytest.approx(1.0, abs=1e-2)

    except NNGException as err:
        raise Exception(str(err))


async def set_vtol_mode_async(drone):
    set_mode = await drone.set_vtol_mode_async(Drone.VTOLMode.FixedWing)
    await set_mode


def test_set_vtol_mode_async(client, reset_scene_drone):
    try:
        drone = reset_scene_drone

        assert drone.enable_api_control() is True
        asyncio.run(set_vtol_mode_async(drone))
        kin = drone.get_ground_truth_kinematics()
        q = kin["pose"]["orientation"]
        qn = math.sqrt(q["w"] ** 2 + q["x"] ** 2 + q["y"] ** 2 + q["z"] ** 2)
        assert qn == pytest.approx(1.0, abs=1e-2)

    except NNGException as err:
        raise Exception(str(err))


def test_set_external_force(client, reset_scene_drone):
    try:
        drone = reset_scene_drone
        start_pose = drone.get_ground_truth_pose()["translation"]

        # drone should take off
        assert drone.set_external_force([0, 0, -10]) is True
        time.sleep(2)
        curr_pose = drone.get_ground_truth_pose()["translation"]
        assert start_pose["x"] == curr_pose["x"]
        assert start_pose["y"] == curr_pose["y"]
        assert start_pose["z"] > curr_pose["z"]

    except NNGException as err:
        raise Exception(str(err))


def test_get_surface_elevation_at_point(client, scene_world):
    try:
        p0 = scene_world.get_surface_elevation_at_point(0, 0)
        p1 = scene_world.get_surface_elevation_at_point(-10, 5)
        p2 = scene_world.get_surface_elevation_at_point(21, -35)
        p3 = scene_world.get_surface_elevation_at_point(71, -64)

        # Validate API returns finite, repeatable elevations without assuming a fixed map.
        for z in (p0, p1, p2, p3):
            assert math.isfinite(z)
            assert -10000.0 < z < 10000.0

        p0_repeat = scene_world.get_surface_elevation_at_point(0, 0)
        assert p0_repeat == pytest.approx(p0, abs=0.05)
    except NNGException as err:
        raise Exception(str(err))


# def test_debug_image_integrity(drone):
#     """
#     This debug test is for checking image data integrity for the client API
#     get_images() requesting captured image data from the sim camera sensor
#     without getting corrupted by new image capture data being moved from
#     UUnrealCamera::OnRendered() to the sim camera through
#     Camera::Impl::PublishImages().
#     Before running this test, in UnrealCamera.h set TEST_DEBUG_IMAGE_INTEGRITY to 1
#     and rebuild ProjectAirSim to enable the logic to overwrite every captured image's
#     pixel BGR values to a single debug constant value (0-255) that increments as
#     each image is captured. If each image received by the client has consistent
#     values for all of its pixel values, then the data is not being corrupted by
#     the next captured images.
#     """
#     try:
#         for i in range(100):
#             image = drone.get_images(
#                 camera_id="DownCamera", image_type_ids=[ImageType.SCENE]
#             )
#             image_data = image[ImageType.SCENE]["data"]
#             image_val = image_data[0]
#             for x in image_data:
#                 assert x == image_val
#             print(f"image OK for val={image_val}")
#     except NNGException as err:
#         raise Exception(str(err))
