"""
Copyright (C) Microsoft Corporation. All rights reserved.
End-to-end tests for ProjectAirSim Services, request-response APIs
"""

from math import radians
import time

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
    world = World(client, "scene_test_drone_sensors.jsonc", 1)
    return world


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


def test_get_imu_data(drone):
    try:
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

    except NNGException as err:
        raise Exception(str(err))


def test_get_barometer_data(drone):
    try:
        barometer_data = drone.get_barometer_data("Barometer")
        print(f"barometer_data:{barometer_data}")
        expected_fields = ["time_stamp", "altitude", "pressure", "qnh"]
        actual_fields = barometer_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)

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

    except NNGException as err:
        raise Exception(str(err))

def test_get_airspeed_data(drone):
    try:
        airspeed_data = drone.get_airspeed_data("Airspeed")
        print(f"airspeed_data:{airspeed_data}")
        expected_fields = [
            "time_stamp",
            "diff_pressure",
        ]
        actual_fields = airspeed_data.keys()
        assert sorted(expected_fields) == sorted(actual_fields)

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
