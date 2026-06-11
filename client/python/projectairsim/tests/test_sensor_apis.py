"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
End-to-end tests for ProjectAirSim Services, request-response APIs
"""

from math import radians
import asyncio
import math
import time

import numpy as np
import pytest

import projectairsim.utils as utils
from pynng import NNGException
from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.types import ImageType
from projectairsim.utils import quaternion_to_rpy


@pytest.fixture(scope="module", autouse=True)
def client(request):
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
def drone(client, world):
    # name should be in actors[*]["name"] in scene_drone_sensors.jsonc
    name = "Drone1"
    return Drone(client, world, name)


@pytest.fixture(scope="module")
def world(client):
    # Create world with retries for scene loading
    # Scene loading can timeout if the simulator is busy or the scene is complex
    max_retries = 2
    last_error = None
    for attempt in range(max_retries):
        try:
            print(f"\\nAttempt {attempt + 1}/{max_retries}: Loading scene_test_drone_sensors.jsonc...")
            world = World(client, "scene_test_drone_sensors.jsonc", 1)
            print("Scene loaded successfully")
            return world
        except RuntimeError as e:
            last_error = e
            error_msg = str(e)
            if "Timeout" in error_msg or "timeout" in error_msg.lower():
                print(f"Scene load timeout (attempt {attempt + 1}/{max_retries}): {error_msg}")
                if attempt < max_retries - 1:
                    print("Waiting 3 seconds before retry...")
                    time.sleep(3.0)
                    continue
            else:
                # Non-timeout error, raise immediately
                raise
    # If we get here, all retries failed
    pytest.skip(f"Failed to load scene after {max_retries} attempts. Last error: {last_error}")


def test_sensor_timestamp_validity(drone):
    """Assert monotonic timestamp updates"""
    try:
        imu_data_t1 = drone.get_imu_data("IMU1")
        t1 = imu_data_t1["time_stamp"]
        print(f"imu_data1[time_stamp]:{t1}")
        time.sleep(1)

        imu_data_t2 = drone.get_imu_data("IMU1")
        t2 = imu_data_t2["time_stamp"]
        print(f"imu_data2[time_stamp]:{t2}")
        time.sleep(4e-3)  # Sim is expected to tick in 3e-3 seconds

        imu_data_t3 = drone.get_imu_data("IMU1")
        t3 = imu_data_t3["time_stamp"]
        print(f"imu_data3[time_stamp]:{t3}")
        assert t3 > t2 > t1

    except NNGException as err:
        raise Exception(str(err))


def test_get_imu_data(drone, world):
    try:
        world.pause()
        imu_data = drone.get_imu_data("IMU1")
        print(f"imu_data:{imu_data}")
        expected_fields = [
            "time_stamp",
            "orientation",
            "angular_velocity",
            "linear_acceleration",
        ]
        actual_fields = imu_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)
        assert sorted(["w", "x", "y", "z"]) == sorted(
            utils.decode(imu_data["orientation"].keys())
        )
        assert sorted(["x", "y", "z"]) == sorted(
            utils.decode(imu_data["angular_velocity"].keys())
        )
        assert sorted(["x", "y", "z"]) == sorted(
            utils.decode(imu_data["linear_acceleration"].keys())
        )

        kin = drone.get_ground_truth_kinematics()
        q_imu = imu_data["orientation"]
        q_gt = kin["pose"]["orientation"]
        r_imu = quaternion_to_rpy(q_imu["w"], q_imu["x"], q_imu["y"], q_imu["z"])
        r_gt = quaternion_to_rpy(q_gt["w"], q_gt["x"], q_gt["y"], q_gt["z"])
        for a, b in zip(r_imu, r_gt):
            assert a == pytest.approx(b, abs=0.08)
        world.resume()

    except NNGException as err:
        raise Exception(str(err))


def test_get_gps_data(drone):
    try:
        gps_data = drone.get_gps_data("GPS")
        print(f"gps_data:{gps_data}")
        expected_fields = [
            "time_stamp",
            "time_utc_millis",
            "latitude",
            "longitude",
            "altitude",
            "epv",
            "eph",
            "position_cov_type",
            "fix_type",
            "velocity",
        ]
        actual_fields = gps_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)
        assert sorted(["x", "y", "z"]) == sorted(
            utils.decode(gps_data["velocity"].keys())
        )

        loc = drone.get_ground_truth_geo_location()
        assert gps_data["latitude"] == pytest.approx(loc["latitude"], abs=1e-4)
        assert gps_data["longitude"] == pytest.approx(loc["longitude"], abs=1e-4)
        assert gps_data["altitude"] == pytest.approx(loc["altitude"], abs=2.0)

    except NNGException as err:
        raise Exception(str(err))


def test_get_barometer_data(drone, world):
    try:
        world.pause()
        barometer_data = drone.get_barometer_data("Barometer")
        print(f"barometer_data:{barometer_data}")
        expected_fields = ["time_stamp", "altitude", "pressure", "qnh"]
        actual_fields = barometer_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)

        loc = drone.get_ground_truth_geo_location()
        assert barometer_data["altitude"] == pytest.approx(loc["altitude"], abs=15.0)
        assert 50000.0 < barometer_data["pressure"] < 120000.0
        world.resume()

    except NNGException as err:
        raise Exception(str(err))


def test_get_magnetometer_data(drone):
    try:
        magnetometer_data = drone.get_magnetometer_data("Magnetometer")
        print(f"magnetometer_data:{magnetometer_data}")
        expected_fields = [
            "time_stamp",
            "magnetic_field_body",
            "magnetic_field_covariance",
        ]
        actual_fields = magnetometer_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)
        assert sorted(["x", "y", "z"]) == sorted(
            utils.decode(magnetometer_data["magnetic_field_body"].keys())
        )
        b = utils.decode(magnetometer_data["magnetic_field_body"])
        bn = math.hypot(float(b["x"]), float(b["y"]), float(b["z"]))
        assert bn > 1e-6

    except NNGException as err:
        raise Exception(str(err))

def test_get_airspeed_data(drone, world):
    try:
        world.pause()
        airspeed_data = drone.get_airspeed_data("Airspeed")
        print(f"airspeed_data:{airspeed_data}")
        expected_fields = [
            "time_stamp",
            "diff_pressure",
        ]
        actual_fields = airspeed_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)
        assert abs(airspeed_data["diff_pressure"]) < 500.0
        world.resume()

    except NNGException as err:
        raise Exception(str(err))

def test_camera_pose(drone, world):
    try:
        world.pause()

        drone_kin = drone.get_ground_truth_kinematics()
        drone_pos = drone_kin["pose"]["position"]
        drone_rot = drone_kin["pose"]["orientation"]
        drone_roll, drone_pitch, drone_yaw = quaternion_to_rpy(
            drone_rot["w"], drone_rot["x"], drone_rot["y"], drone_rot["z"]
        )

        images = drone.get_images(
            camera_id="DownCamera", image_type_ids=[ImageType.SCENE]
        )
        pos_x = images[ImageType.SCENE]["pos_x"]
        pos_y = images[ImageType.SCENE]["pos_y"]
        pos_z = images[ImageType.SCENE]["pos_z"]
        rot_w = images[ImageType.SCENE]["rot_w"]
        rot_x = images[ImageType.SCENE]["rot_x"]
        rot_y = images[ImageType.SCENE]["rot_y"]
        rot_z = images[ImageType.SCENE]["rot_z"]
        roll, pitch, yaw = quaternion_to_rpy(rot_w, rot_x, rot_y, rot_z)

        relative_x = pos_x - drone_pos["x"]
        relative_y = pos_y - drone_pos["y"]
        relative_z = pos_z - drone_pos["z"]
        relative_roll = roll - drone_roll
        relative_pitch = pitch - drone_pitch
        relative_yaw = yaw - drone_yaw
        print(
            f"rel. x (m) = {relative_x}, "
            f"rel. y (m) = {relative_y}, "
            f"rel. z (m) = {relative_z}"
        )
        print(
            f"rel. pitch (rad) = {relative_pitch}, "
            f"rel. roll (rad) = {relative_roll}, "
            f"rel. yaw (rad) = {relative_yaw}"
        )

        # Check against camera setting origin setting values from the
        # robot_quadrotor_fastphysics_sensors.jsonc config file loaded by
        # scene_drone_sensors.jsonc in this test script
        tol_pos = 0.001
        assert relative_x == pytest.approx(1.1, tol_pos)
        assert relative_y == pytest.approx(2.2, tol_pos)
        assert relative_z == pytest.approx(-3.3, tol_pos)
        tol_rot = radians(0.001)
        assert relative_pitch == pytest.approx(radians(-85.94), tol_rot)
        assert relative_roll == pytest.approx(radians(5.73), tol_rot)
        assert relative_yaw == pytest.approx(radians(11.46), tol_rot)

        world.resume()

    except NNGException as err:
        raise Exception(str(err))


def test_camera_look_at_object(drone, world):
    try:
        drone.camera_look_at_object(camera_id="DownCamera", object_name="OrangeBall")

        images = drone.get_images(
            camera_id="DownCamera", image_type_ids=[ImageType.SCENE]
        )

        img_width = images[ImageType.SCENE]["width"]
        img_height = images[ImageType.SCENE]["height"]

        # Check that OrangeBall is the only annotated object
        assert len(images[ImageType.SCENE]["annotations"]) == 1

        bbox_center = images[ImageType.SCENE]["annotations"][0]["bbox2d"]["center"]
        pos_x = bbox_center["x"]
        pos_y = bbox_center["y"]

        # Check that the OrangeBall is in the middle of the image
        tol_pixel = 2
        assert pos_x == pytest.approx(img_width // 2, tol_pixel)
        assert pos_y == pytest.approx(img_height // 2, tol_pixel)

    except NNGException as err:
        raise Exception(str(err))


# ---------------------------------------------------------------------------
# Flight helpers
# ---------------------------------------------------------------------------


async def _takeoff(drone):
    """Arm and take off. Call _land() when done."""
    drone.enable_api_control()
    drone.arm()
    await (await drone.takeoff_async())


async def _land(drone):
    """Land and disarm."""
    await (await drone.land_async())
    drone.disarm()
    drone.disable_api_control()


# ---------------------------------------------------------------------------
# IMU — gravity consistency
# ---------------------------------------------------------------------------


async def test_imu_gravity_on_ground(drone, world):
    """While stationary on the ground the accelerometer norm should equal 9.81 m/s²."""
    try:
        world.pause()
        imu_data = drone.get_imu_data("IMU1")
        a = utils.decode(imu_data["linear_acceleration"])
        ax, ay, az = float(a["x"]), float(a["y"]), float(a["z"])
        norm = math.hypot(ax, ay, az)
        print(f"accel norm on ground: {norm:.4f} m/s²")
        assert abs(norm - 9.81) < 0.1, f"Expected ~9.81 m/s², got {norm:.4f}"
        world.resume()
    except NNGException as err:
        raise Exception(str(err))


async def test_imu_gravity_in_air(drone):
    """While hovering the accelerometer norm should still equal 9.81 m/s²."""
    try:
        await _takeoff(drone)
        imu_data = drone.get_imu_data("IMU1")
        a = utils.decode(imu_data["linear_acceleration"])
        ax, ay, az = float(a["x"]), float(a["y"]), float(a["z"])
        norm = math.hypot(ax, ay, az)
        print(f"accel norm in air: {norm:.4f} m/s²")
        assert abs(norm - 9.81) < 0.1, f"Expected ~9.81 m/s², got {norm:.4f}"
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)


# ---------------------------------------------------------------------------
# IMU — pure yaw gyro response
# ---------------------------------------------------------------------------


async def test_imu_pure_yaw(drone):
    """During a slow 360° yaw: gyro-Z non-zero, gyro-X/Y near zero."""
    try:
        await _takeoff(drone)
        # 2π/10 rad/s → full 360° in 10 s
        yaw_rate = 2 * math.pi / 10
        rotate_task = await drone.rotate_by_yaw_rate_async(
            yaw_rate=yaw_rate, duration=10.0
        )
        # Sample mid-rotation
        await asyncio.sleep(5.0)
        imu_data = drone.get_imu_data("IMU1")
        omega = utils.decode(imu_data["angular_velocity"])
        ox = float(omega["x"])
        oy = float(omega["y"])
        oz = float(omega["z"])
        print(f"gyro x={ox:.4f}, y={oy:.4f}, z={oz:.4f} rad/s")
        assert abs(oz) > 0.1, f"Expected non-zero gyro-Z during yaw, got {oz:.4f}"
        assert abs(ox) < 0.3, f"Expected gyro-X near zero, got {ox:.4f}"
        assert abs(oy) < 0.3, f"Expected gyro-Y near zero, got {oy:.4f}"
        await rotate_task
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)


# ---------------------------------------------------------------------------
# Magnetometer — 360° heading monotonic
# ---------------------------------------------------------------------------


async def test_mag_heading_monotonic_during_yaw(drone):
    """During a full 360° yaw the decoded heading should change monotonically."""
    try:
        await _takeoff(drone)
        yaw_rate = 2 * math.pi / 10  # rad/s → full 360° in 10 s
        rotate_task = await drone.rotate_by_yaw_rate_async(
            yaw_rate=yaw_rate, duration=10.0
        )
        headings = []
        start = time.monotonic()
        while time.monotonic() - start < 10.0:
            mag = drone.get_magnetometer_data("Magnetometer")
            b = utils.decode(mag["magnetic_field_body"])
            heading = math.atan2(float(b["x"]), float(b["y"]))
            headings.append(heading)
            await asyncio.sleep(0.1)
        await rotate_task
        unwrapped = np.unwrap(headings)
        diffs = np.diff(unwrapped)
        total_sweep = abs(unwrapped[-1] - unwrapped[0])
        print(f"heading total sweep: {math.degrees(total_sweep):.1f}°")
        assert total_sweep >= math.radians(350), (
            f"Expected ≥350° sweep, got {math.degrees(total_sweep):.1f}°"
        )
        assert np.all(diffs > 0) or np.all(diffs < 0), (
            "Heading did not change monotonically during 360° yaw"
        )
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)


# ---------------------------------------------------------------------------
# GPS — straight-line motion accuracy
# ---------------------------------------------------------------------------


async def test_gps_straight_line_motion(drone):
    """Move 10 m north; GPS north-delta should match ground-truth within 2 m."""
    try:
        await _takeoff(drone)
        gps0 = drone.get_gps_data("GPS")
        gt0 = drone.get_ground_truth_kinematics()
        await (
            await drone.move_to_position_async(
                north=10.0, east=0.0, down=-5.0, velocity=2.0, timeout_sec=20.0
            )
        )
        gps1 = drone.get_gps_data("GPS")
        gt1 = drone.get_ground_truth_kinematics()
        gps_north_delta = (gps1["latitude"] - gps0["latitude"]) * 111111.0
        gt_north_delta = gt1["pose"]["position"]["x"] - gt0["pose"]["position"]["x"]
        print(
            f"GPS north Δ: {gps_north_delta:.3f} m, GT north Δ: {gt_north_delta:.3f} m"
        )
        assert abs(gps_north_delta - gt_north_delta) < 2.0, (
            f"GPS/GT north delta mismatch: {abs(gps_north_delta - gt_north_delta):.3f} m"
        )
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)


# ---------------------------------------------------------------------------
# GPS — velocity sign/magnitude consistency
# ---------------------------------------------------------------------------


async def test_gps_velocity_consistency(drone):
    """Command 3 m/s north for 20 s; finite-diff GPS lat should match commanded velocity."""
    try:
        await _takeoff(drone)
        # Start 20 s northward move and sample twice 1 s apart at early-steady state
        move_task = await drone.move_by_velocity_async(
            v_north=3.0, v_east=0.0, v_down=0.0, duration=20.0
        )
        await asyncio.sleep(0.5)
        gps_t1 = drone.get_gps_data("GPS")
        await asyncio.sleep(1.0)
        gps_t2 = drone.get_gps_data("GPS")
        # Finite-difference northward velocity from GPS latitude
        v_fd = (gps_t2["latitude"] - gps_t1["latitude"]) * 111111.0 / 1.0
        print(f"GPS finite-diff v_north: {v_fd:.3f} m/s (commanded 3.0 m/s)")
        assert v_fd > 0, "Expected positive (northward) finite-diff GPS velocity"
        assert abs(v_fd - 3.0) < 2.0, (
            f"GPS velocity magnitude too far from commanded: {v_fd:.3f} vs 3.0 m/s"
        )
        assert gps_t2["velocity"]["x"] > 0, "GPS velocity x field should be positive"
        drone.cancel_last_task()
        await move_task
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)


# ---------------------------------------------------------------------------
# Barometer — ascend/descend altitude tracking
# ---------------------------------------------------------------------------


async def test_barometer_altitude_ascend_descend(drone):
    """Ascend from z=-5 m to z=-20 m; barometer altitude should track GT within 5 m."""
    try:
        await _takeoff(drone)
        # Settle at a low hover altitude
        await (
            await drone.move_to_position_async(
                north=0.0, east=0.0, down=-5.0, velocity=2.0, timeout_sec=15.0
            )
        )
        baro_low = drone.get_barometer_data("Barometer")
        gt_low = drone.get_ground_truth_kinematics()
        # Ascend
        await (
            await drone.move_to_position_async(
                north=0.0, east=0.0, down=-20.0, velocity=2.0, timeout_sec=20.0
            )
        )
        baro_high = drone.get_barometer_data("Barometer")
        gt_high = drone.get_ground_truth_kinematics()
        # NED: z is negative when above origin → altitude gain = -(z_high) - -(z_low)
        gt_alt_change = (
            -gt_high["pose"]["position"]["z"] - (-gt_low["pose"]["position"]["z"])
        )
        baro_alt_change = baro_high["altitude"] - baro_low["altitude"]
        print(f"baro Δalt: {baro_alt_change:.2f} m, GT Δalt: {gt_alt_change:.2f} m")
        assert baro_high["altitude"] > baro_low["altitude"], (
            "Barometer altitude did not increase after ascending"
        )
        assert abs(baro_alt_change - gt_alt_change) < 5.0, (
            f"Baro/GT altitude change mismatch: {abs(baro_alt_change - gt_alt_change):.2f} m"
        )
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)


# ---------------------------------------------------------------------------
# Airspeed — hover near-zero + monotonic ramp
# ---------------------------------------------------------------------------


async def test_airspeed_ramp(drone):
    """diff_pressure near zero at hover; increases strictly monotonically with forward speed."""
    try:
        await _takeoff(drone)
        await asyncio.sleep(1.0)  # settle hover
        dp_hover = drone.get_airspeed_data("Airspeed")["diff_pressure"]
        print(f"diff_pressure at hover: {dp_hover:.4f} Pa")
        assert abs(dp_hover) < 10.0, (
            f"Expected near-zero diff_pressure at hover, got {dp_hover:.4f} Pa"
        )
        dp_values = []
        for v_fwd in [2.0, 4.0, 6.0]:
            await (
                await drone.move_by_velocity_body_frame_async(
                    v_forward=v_fwd, v_right=0.0, v_down=0.0, duration=1.0
                )
            )
            dp = drone.get_airspeed_data("Airspeed")["diff_pressure"]
            print(f"diff_pressure at {v_fwd} m/s: {dp:.4f} Pa")
            dp_values.append(dp)
        assert dp_hover < dp_values[0] < dp_values[1] < dp_values[2], (
            f"diff_pressure not strictly monotonic: hover={dp_hover:.4f}, "
            f"2m/s={dp_values[0]:.4f}, 4m/s={dp_values[1]:.4f}, 6m/s={dp_values[2]:.4f}"
        )
    except NNGException as err:
        raise Exception(str(err))
    finally:
        await _land(drone)
