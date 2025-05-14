"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
End-to-end tests for ProjectAirSim Services, request-response APIs
"""

import asyncio
from datetime import datetime
import math
import time
import random

from pynng import NNGException
import pytest
from typing import Dict

from projectairsim import Drone, ProjectAirSimClient, World, EnvActor
from projectairsim.image_utils import segmentation_id_to_color, segmentation_color_to_id
from projectairsim.utils import geo_to_ned_coordinates
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

PI = 3.14159

# Position tolerance for validation
POSITION_TOLERANCE = 0.1
# Time tolerance for validation
TIME_TOLERANCE = 0.02

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
def world(client):
    # Set up the simulation world
    world = World(client, "scene_test_env_actors_grounded.jsonc", delay_after_load_sec=2, actual_load=True)
    yield world

@pytest.fixture(scope="module")
def env_actor1(client, world):
    yield EnvActor(client, world, "car1")

@pytest.fixture(scope="module")
def env_actor2(client, world):
    yield EnvActor(client, world, "car2")

@pytest.fixture(scope="module")
def env_actor3(client, world):
    yield EnvActor(client, world, "pedestrian1")

@pytest.fixture(scope="module")
def env_actor4(client, world):
    yield EnvActor(client, world, "pedestrian2")

class TrajectoryManager:
    def __init__(self):
        self.trajectories = {}

    # Set start time, start and end positions for a specific actor
    def set_trajectory_(self, actor_name, start_time, start_position, end_position, total_duration):
        self.trajectories[actor_name] = {
            'start_time': start_time,
            'start_position': start_position,
            'end_position': end_position,
            'total_duration': total_duration
        }

    # Get trajectory data for a specific actor
    def get_trajectory(self, actor_name):
        return self.trajectories.get(actor_name, None)

    # Verify the actor's position at a given time against the expected trajectory
    def verify_position(self, current_time, actor_name, msg):
        # Retrieve the stored trajectory data for the given actor
        trajectory = self.get_trajectory(actor_name)
        if not trajectory:
            return False

        start_time = trajectory['start_time']
        start_pos = trajectory['start_position']
        end_pos = trajectory['end_position']
        total_duration = trajectory['total_duration']

        actual_x = msg['kinematics']['pose']['position']['x']
        actual_y = msg['kinematics']['pose']['position']['y']

        # Check position at the start of the trajectory
        if current_time - start_time <= TIME_TOLERANCE:
            assert abs(actual_x - start_pos['x']) <= POSITION_TOLERANCE, f"Position mismatch at start for {actor_name}. Expected: {start_pos['x']}, Got: {actual_x}"
            assert abs(actual_y - start_pos['y']) <= POSITION_TOLERANCE, f"Position mismatch at start for {actor_name}. Expected: {start_pos['y']}, Got: {actual_y}"
        # Check position at the midpoint of the trajectory
        elif abs(current_time - (start_time + total_duration / 2)) <= TIME_TOLERANCE:
            mid_x = (start_pos['x'] + end_pos['x']) / 2
            mid_y = (start_pos['y'] + end_pos['y']) / 2
            assert abs(actual_x - mid_x) <= POSITION_TOLERANCE, f"Position mismatch at midpoint for {actor_name}. Expected: {mid_x}, Got: {actual_x}"
            assert abs(actual_y - mid_y) <= POSITION_TOLERANCE, f"Position mismatch at midpoint for {actor_name}. Expected: {mid_y}, Got: {actual_y}"
        # Check position at the end of the trajectory
        elif abs(current_time - (start_time + total_duration)) <= TIME_TOLERANCE:
            assert abs(actual_x - end_pos['x']) <= POSITION_TOLERANCE, f"Position mismatch at end for {actor_name}. Expected: {end_pos['x']}, Got: {actual_x}"
            assert abs(actual_y - end_pos['y']) <= POSITION_TOLERANCE, f"Position mismatch at end for {actor_name}. Expected: {end_pos['y']}, Got: {actual_y}"

        return True

def set_env_car_traj(actor, world, x_offset=0, y_offset=0, time_offset=0):
    # Set up the trajectory and get the initial time
    start_time = world.get_sim_time() / 1e9
    time_sec = [start_time, start_time + 30]
    pose_x = [100, 200]
    pose_y = [100, 200]
    pose_z = [-2, -2]
    pose_yaw = [0, PI/2]
    pose_roll = [0] * len(time_sec)
    pose_pitch = [0] * len(time_sec)

    # Import the trajectory into the environment
    traj_name = "traj_with_offset" if (x_offset or y_offset or time_offset) else "traj"
    world.import_ned_trajectory(traj_name, time_sec, pose_x, pose_y, pose_z, pose_roll, pose_pitch, pose_yaw)

    # Assign the trajectory to the actor with the specified offsets
    actor.set_trajectory(traj_name, to_loop=False, time_offset=time_offset, x_offset=x_offset, y_offset=y_offset)

    time.sleep(1)  # Wait to process the trajectory

    # Store the start and end positions
    start_position = {'x': pose_x[0] + x_offset, 'y': pose_y[0] + y_offset}
    end_position = {'x': pose_x[-1] + x_offset, 'y': pose_y[-1] + y_offset}
    total_duration = time_sec[-1] - time_sec[0] + time_offset

    return start_position, end_position, start_time, total_duration

def kinematics_callback(world, actor_name, msg, traj_manager):
    # Handle the kinematics callback and verify the actor's position
    current_time = world.get_sim_time() / 1e9
    if not traj_manager.verify_position(current_time, actor_name, msg):
        print(f"Verification failed for {actor_name} at time {current_time}")

@pytest.fixture(scope="module")
def set_trajectories(env_actor1, env_actor2, env_actor3, env_actor4, world):
    # Initialize the trajectory manager to store and manage trajectory data
    traj_manager = TrajectoryManager()

    # Set up trajectories for car1 and pedestrian1 (without offset)
    start_pos1, end_pos1, start_time1, total_duration1 = set_env_car_traj(env_actor1, world)
    start_pos3, end_pos3, start_time3, total_duration3 = set_env_car_traj(env_actor3, world, 2, 2)

    traj_manager.set_trajectory_("car1", start_time1, start_pos1, end_pos1, total_duration1)
    traj_manager.set_trajectory_("pedestrian1", start_time3, start_pos3, end_pos3, total_duration3)

    # Set up trajectories for car2 and pedestrian2 (with offset)
    X_OFFSET = random.uniform(-50, 50)
    Y_OFFSET = random.uniform(-50, 50)
    TIME_OFFSET = random.uniform(1, 60)

    start_pos2, end_pos2, start_time2, total_duration2 = set_env_car_traj(env_actor2, world, X_OFFSET, Y_OFFSET, TIME_OFFSET)
    start_pos4, end_pos4, start_time4, total_duration4 = set_env_car_traj(env_actor4, world, X_OFFSET+2, Y_OFFSET+2, TIME_OFFSET)

    traj_manager.set_trajectory_("car2", start_time2, start_pos2, end_pos2, total_duration2)
    traj_manager.set_trajectory_("pedestrian2", start_time4, start_pos4, end_pos4, total_duration4)

    return traj_manager

def test_enable_disable_api_control(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_arm_disarm(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_takeoff_and_land_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_takeoff_and_hover_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_move_by_velocity_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_at_z = await drone.move_by_velocity_z_async(
        v_north=2.0, v_east=0.0, z=-5.0, duration=2.0
    )
    await move_at_z


def test_move_by_velocity_z_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


async def move_by_velocity_body_frame_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_north_up = await drone.move_by_velocity_body_frame_async(
        v_forward=2.0, v_right=0.0, v_down=-1.0, duration=2.0
    )
    await move_north_up


def test_move_by_velocity_body_frame_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_at_z = await drone.move_by_velocity_body_frame_z_async(
        v_forward=2.0, v_right=0.0, z=-5.0, duration=2.0
    )
    await move_at_z


def test_move_by_velocity_body_frame_z_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


async def move_by_heading_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    move_by_heading = await drone.move_by_heading_async(heading=90.0, speed=5.0)
    await move_by_heading


def test_move_by_heading_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_move_to_position_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_move_to_geo_position_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_go_home_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(go_home_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_on_path_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    path = [[3, 3, -6], [3, -3, -6]]
    move_on_path = await drone.move_on_path_async(path, velocity=1.0)
    await move_on_path


def test_move_on_path_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        api_control_enabled = drone.enable_api_control()
        assert api_control_enabled is True
        armed = drone.arm()
        assert armed is True
        asyncio.run(move_on_path_async(drone))
        disarmed = drone.disarm()
        assert disarmed is True
        api_control_disabled = drone.disable_api_control()
        assert api_control_disabled is True
    except NNGException as err:
        raise Exception(str(err))


async def move_on_geo_path_async(drone):
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    path = [[47.641460, -122.140160, 125.0], [47.641470, -122.140165, 126.0]]
    move_on_path = await drone.move_on_geo_path_async(path, velocity=1.0)
    await move_on_path


def test_move_on_geo_path_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_rotate_yaw_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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
    # basic test for now to make sure no exceptions are thrown
    take_off = await drone.takeoff_async()
    await take_off

    rotate = await drone.rotate_by_yaw_rate_async(yaw=3.14)
    await rotate


def test_rotate_by_yaw_rate_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_get_sim_clock_type(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        clock_type = world.get_sim_clock_type()
        assert "unknown" not in clock_type
    except NNGException as err:
        raise Exception(str(err))


def test_get_sim_time(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        sim_time = world.get_sim_time()
        assert sim_time is not None
        print(f"Received: sim_time={sim_time * 1e-9} seconds")
    except NNGException as err:
        raise Exception(str(err))


def test_pause_resume(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        clock_type = world.get_sim_clock_type()
        sim_pause_resume_state: str = world.pause()
        sim_time = world.get_sim_time()
        time.sleep(0.1)
        if "steppable" in clock_type:
            assert "paused" in sim_pause_resume_state
            assert world.get_sim_time() == sim_time
        elif "real-time" in clock_type:
            assert "WARNING" in sim_pause_resume_state
            assert world.get_sim_time() > sim_time

        sim_pause_resume_state: str = world.resume()
        if "steppable" in clock_type:
            assert "resumed" in sim_pause_resume_state
        elif "real-time" in clock_type:
            assert "WARNING" in sim_pause_resume_state
        sim_time = world.get_sim_time()
        time.sleep(0.1)
        assert world.get_sim_time() > sim_time
    except NNGException as err:
        raise Exception(str(err))


def test_is_paused(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        clock_type = world.get_sim_clock_type()
        world.pause()
        if "steppable" in clock_type:
            assert world.is_paused() is True
        elif "real-time" in clock_type:
            assert world.is_paused() is False

        world.resume()
        assert world.is_paused() is False
    except NNGException as err:
        raise Exception(str(err))


def test_continue_for_sim_time(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        clock_type = world.get_sim_clock_type()
        world.pause()
        sim_time = world.get_sim_time()
        delta_time = 5 * 3e6  # TODO Has to be a multiple of JSON step ns
        world.continue_for_sim_time(delta_time)
        if "steppable" in clock_type:
            assert world.get_sim_time() == sim_time + delta_time
        elif "real-time" in clock_type:
            assert world.get_sim_time() >= sim_time
        world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_continue_until_sim_time(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        clock_type = world.get_sim_clock_type()
        world.pause()
        sim_time = world.get_sim_time()
        target_time = sim_time + 5 * 3e6  # TODO Has to be a multiple of JSON step ns
        world.continue_until_sim_time(target_time)
        if "steppable" in clock_type:
            assert world.get_sim_time() == target_time
        elif "real-time" in clock_type:
            assert world.get_sim_time() >= target_time
        world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_continue_for_n_steps(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        clock_type = world.get_sim_clock_type()
        world.pause()
        world.continue_for_n_steps(1)  # Step to next multiple of the step size
        sim_time = world.get_sim_time()
        step_time = 3e6  # TODO This is JSON config dependent
        world.continue_for_n_steps(1)
        if "steppable" in clock_type:
            assert world.get_sim_time() == sim_time + step_time * 1
        elif "real-time" in clock_type:
            assert world.get_sim_time() >= sim_time

        sim_time = world.get_sim_time()
        world.continue_for_n_steps(3)
        if "steppable" in clock_type:
            assert world.get_sim_time() == sim_time + step_time * 3
        elif "real-time" in clock_type:
            assert world.get_sim_time() >= sim_time
        world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_continue_for_single_step(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        clock_type = world.get_sim_clock_type()
        world.pause()
        world.continue_for_single_step()  # Step to next multiple of the step size
        sim_time = world.get_sim_time()
        step_time = 3e6  # TODO This is JSON config dependent
        world.continue_for_single_step()
        if "steppable" in clock_type:
            assert world.get_sim_time() == sim_time + step_time * 1
        elif "real-time" in clock_type:
            assert world.get_sim_time() >= sim_time
        world.resume()
    except NNGException as err:
        raise Exception(str(err))


def test_list_actors(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        drone = Drone(client, world, "Drone1")

        actor_ids = world.list_actors()
        print(f"Received actor_ids: {actor_ids}")
        assert drone.name in actor_ids
    except NNGException as err:
        raise Exception(str(err))


def test_list_assets(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        asset_ids = world.list_assets(".*")
        print(f"Received actor_ids: {asset_ids}")
        # Assets required by other tests
        assert "BasicLandingPad" in asset_ids
        assert "OrangeBall_Blueprint" in asset_ids
    except NNGException as err:
        raise Exception(str(err))


def test_list_objects(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        name_regex = ".*"
        scene_objects_list = world.list_objects(name_regex)
        print("object_list:", scene_objects_list)
        assert len(scene_objects_list)

    except NNGException as err:
        raise Exception(str(err))


def test_get_object_pose(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        object_id: str = "OrangeBall"  # Present in Blocks env
        object_pose = world.get_object_pose(object_id)
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


def test_get_object_poses(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        # "OrangeBall" is a scene object in Blocks env, "Drone1" is the spawned robot
        # that can also be queried as an object
        object_ids = ["OrangeBall", "Drone1"]
        object_poses = world.get_object_poses(object_ids)
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


def test_set_object_pose(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        object_query: str = "OrangeBall"  # Present in Blocks env
        objects = world.list_objects(object_query + ".*")
        assert len(objects)
        object_id = objects[0]
        orig_pose = world.get_object_pose(object_id)
        ref_frame = "DEFAULT_ID"
        translation = Vector3({"x": 20, "y": 0, "z": 0})
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        new_pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        status = world.set_object_pose(object_id, new_pose, True)
        print("Set pose success:", status)
        assert status
        time.sleep(0.5)
        obtained_pose = world.get_object_pose(object_id)
        assert obtained_pose == new_pose
        world.set_object_pose(object_id, orig_pose, True)  # return to orig pose
    except NNGException as err:
        raise Exception(str(err))


def test_get_object_scale(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        object_id: str = "OrangeBall"  # Present in Blocks env
        object_scale = world.get_object_scale(object_id)
        print("object_scale:", object_scale)
        assert object_scale is not None
        assert not any(
            [math.isnan(x) for x in object_scale]
        )  # Scale returns zeros if object doesn't exist

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_scale(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        object_id: str = "OrangeBall"  # Present in Blocks env
        scale = [10.0, 10.0, 10.0]
        status = world.set_object_scale(object_id, scale)
        print("Set scale success:", status)
        assert status
        new_scale = world.get_object_scale(object_id)
        assert new_scale == scale

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_destroy_object(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TestLandingPad"
        asset_name: str = "BasicLandingPad"  # Existing asset
        ref_frame = "DEFAULT_ID"

        translation = Vector3({"x": 20, "y": 1, "z": -5})
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        scale = [10.0, 10.0, 10.0]
        enable_physics: bool = False
        retval = world.spawn_object(
            object_name, asset_name, pose, scale, enable_physics
        )

        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = world.destroy_object(object_name)
        assert status
        objects = world.list_objects(object_name + ".*")
        assert not len(objects)

        # ------------------------------------------------------------------
        # Check spawning a Blueprint object with its physics disabled
        object_name: str = "TestBPOrangeBall"
        asset_name: str = "OrangeBall_Blueprint"  # Existing asset
        ref_frame = "DEFAULT_ID"

        translation = Vector3({"x": 20, "y": 1, "z": -5})
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        scale = [1.0, 1.0, 1.0]
        enable_physics: bool = False
        retval = world.spawn_object(
            object_name, asset_name, pose, scale, enable_physics
        )

        print("Spawned Blueprint object successfully with name: ", retval)
        new_objs = world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = world.destroy_object(object_name)
        assert status
        objects = world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_object_at_geo(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
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
        retval = world.spawn_object_at_geo(
            object_name, asset_name, lat, lon, alt, rotation, scale, enable_physics
        )

        print("Spawned object successfully with name: ", retval)
        new_objs = world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        object_pose = world.get_object_pose(object_name)
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
        status = world.destroy_object(object_name)
        assert status
        objects = world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_object_from_file(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TestLandingPad"
        asset_name: str = "BasicLandingPad"  # Existing asset
        ref_frame = "DEFAULT_ID"

        translation = Vector3({"x": 20, "y": 1, "z": -5})
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        scale = [10.0, 10.0, 10.0]
        enable_physics: bool = False
        binary_gltf: bool = True
        file1 = open("assets/BasicLandingPad.glb", "rb")
        gltf_byte_array = file1.read()
        file1.close()
        retval = world.spawn_object_from_file(
            object_name,
            "gltf",
            gltf_byte_array,
            binary_gltf,
            pose,
            scale,
            enable_physics,
        )
        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = world.destroy_object(object_name)
        assert status
        objects = world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_spawn_object_from_file_at_geo(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TestLandingPad"
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
        retval = world.spawn_object_from_file_at_geo(
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
        new_objs = world.list_objects(object_name + ".*")
        assert len(new_objs) > 0

        # Check destroying object that was just spawned
        status = world.destroy_object(object_name)
        assert status
        objects = world.list_objects(object_name + ".*")
        assert not len(objects)

    except NNGException as err:
        raise Exception(str(err))


def test_destroy_all_spawned_objects(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        # ------------------------------------------------------------------
        # Check spawning a static mesh object with its physics disabled
        object_name: str = "TestLandingPad"
        asset_name: str = "BasicLandingPad"  # Existing asset
        ref_frame = "DEFAULT_ID"
        for i in range(10):
            translation = Vector3({"x": 20 + i, "y": 1, "z": -5})
            rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
            pose: Pose = Pose(
                {
                    "translation": translation,
                    "rotation": rotation,
                    "frame_id": ref_frame,
                }
            )
            scale = [10.0, 10.0, 10.0]
            enable_physics: bool = False
            retval = world.spawn_object(
                object_name, asset_name, pose, scale, enable_physics
            )

        print("Spawned static mesh object successfully with name: ", retval)
        new_objs = world.list_objects(object_name + ".*")
        assert len(new_objs) >= 10

        # Check destroying object that was just spawned
        status = world.destroy_all_spawned_objects()
        assert status
        objects = world.list_objects(object_name + ".*")
        assert not len(objects)
    except NNGException as err:
        raise Exception(str(err))


def test_enable_disable_weather_visual_effects(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        status = world.enable_weather_visual_effects()
        assert status is True

        status = world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_weather_visual_effects_param(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        status = world.enable_weather_visual_effects()
        assert status is True
        status = world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.9)
        assert status is True
        status = world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_reset_weather_visual_effects(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        status = world.enable_weather_visual_effects()
        assert status is True
        status = world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.9)
        assert status is True
        status = world.reset_weather_effects()
        assert status is True
        status = world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_get_weather_visual_effects(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        status = world.enable_weather_visual_effects()
        assert status is True
        status = world.set_weather_visual_effects_param(WeatherParameter.RAIN, 0.9)
        assert status is True
        weather_dict = world.get_weather_visual_effects_param()
        assert weather_dict[WeatherParameter.RAIN] == pytest.approx(0.9)
        status = world.set_weather_visual_effects_param(WeatherParameter.SNOW, 0.4)
        assert status is True
        weather_dict = world.get_weather_visual_effects_param()
        assert weather_dict[WeatherParameter.RAIN] == pytest.approx(0.9)
        assert weather_dict[WeatherParameter.SNOW] == pytest.approx(0.4)
        status = world.reset_weather_effects()
        assert status is True
        weather_dict = world.get_weather_visual_effects_param()
        assert weather_dict[WeatherParameter.RAIN] == pytest.approx(0)
        assert weather_dict[WeatherParameter.SNOW] == pytest.approx(0)
        status = world.disable_weather_visual_effects()
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_wind_velocity(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        world.set_wind_velocity(5.0, 5.0, 5.0)

        assert world.get_wind_velocity() == pytest.approx((5.0, 5.0, 5.0))

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_material(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        object_name = "OrangeBall"
        material_path = "/ProjectAirSim/Weather/WeatherFX/Materials/M_Leaf_master"
        status = world.set_object_material(object_name, material_path)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_texture_from_url(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        url = "https://www.jpl.nasa.gov/spaceimages/images/largesize/PIA07782_hires.jpg"
        object_name = "OrangeBall"
        status = world.set_object_texture_from_url(object_name, url)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_texture_from_file(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        object_name = "OrangeBall"
        texture_path = "assets/sample_texture.png"
        status = world.set_object_texture_from_file(object_name, texture_path)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_object_texture_from_packaged_asset(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)
        object_name = "Cone"
        texture_path = "/Game/Geometry/Textures/T_Default_Material_Grid_M"
        status = world.set_object_texture_from_packaged_asset(object_name, texture_path)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_swap_object_texture(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        # Note: This API searches for the actor tag instead of the actor's name. The
        # actor tag is set in Editor's actor details under the "Actor" section, not the
        # component tags in the "Tags" section
        object_actor_tag = "ball"
        swapped_objects = world.swap_object_texture(object_actor_tag, 1)
        assert len(swapped_objects) > 0

        swapped_objects = world.swap_object_texture(object_actor_tag, 0)
        assert len(swapped_objects) > 0

    except NNGException as err:
        raise Exception(str(err))
    
def test_set_light_object_intensity(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        spotlight_asset_path = "SpotLightActor"
        ref_frame = "DEFAULT_ID"
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        scale = [1.0, 1.0, 1.0]
        enable_physics = False
        pad2_name = "SpotLight1"
        translation = Vector3({"x": 0.0, "y": 8.0, "z": -5.0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        spot_light_object_name = world.spawn_object(pad2_name, spotlight_asset_path, pose, scale, enable_physics)

        status = world.set_light_object_intensity(spot_light_object_name, 10000.0)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))
    
def test_set_light_object_color(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        spotlight_asset_path = "SpotLightActor"
        ref_frame = "DEFAULT_ID"
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        scale = [1.0, 1.0, 1.0]
        enable_physics = False
        pad2_name = "SpotLight1"
        translation = Vector3({"x": 0.0, "y": 8.0, "z": -5.0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        spot_light_object_name = world.spawn_object(pad2_name, spotlight_asset_path, pose, scale, enable_physics)
        color_rgb = [1.0, 0.0, 0.0]
        status = world.set_light_object_color(spot_light_object_name, color_rgb)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))

def test_set_light_object_radius(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        spot_light_asset_path = "SpotLightActor"
        ref_frame = "DEFAULT_ID"
        rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
        scale = [1.0, 1.0, 1.0]
        enable_physics = False
        pad2_name = "SpotLight1"
        translation = Vector3({"x": 0.0, "y": 8.0, "z": -5.0})
        pose: Pose = Pose(
            {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
        )
        spot_light_object_name = world.spawn_object(pad2_name, spot_light_asset_path, pose, scale, enable_physics)

        status = world.set_light_object_radius(spot_light_object_name, 16000.0)
        assert status is True

        directional_light_asset_path = "DirectionalLightActor"
        directional_light_object_name = world.spawn_object(pad2_name, directional_light_asset_path, pose, scale, enable_physics)

        status = world.set_light_object_radius(directional_light_object_name, 16000.0)
        assert status is False

    except NNGException as err:
        raise Exception(str(err))

def test_set_time_of_day(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        enabled = True
        now_datetime = "2020-01-01 12:00:00"
        move_sun = True
        is_start_datetime_dst = True

        status = world.set_time_of_day(
            enabled, now_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_get_time_of_day(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        enabled = True
        now_datetime = "2020-01-01 12:00:00"
        move_sun = True
        is_start_datetime_dst = True

        status = world.set_time_of_day(
            enabled, now_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )
        assert status is True
        time_str = world.get_time_of_day()
        assert time_str == now_datetime

        now_datetime = "2020-01-01 09:00:00"
        status = world.set_time_of_day(
            enabled, now_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )
        assert status is True
        time_str = world.get_time_of_day()
        assert time_str == now_datetime

    except NNGException as err:
        raise Exception(str(err))


def test_set_sun_position_from_datetime(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        # Stop the time of day feature
        status = False
        cmd_datetime = "2020-01-01 21:00:00"
        move_sun = False
        is_start_datetime_dst = True

        status = world.set_time_of_day(
            status, cmd_datetime, is_start_datetime_dst, 1.0, 1.0, move_sun
        )

        # Try setting to current sun position
        now = datetime.now()

        status = world.set_sun_position_from_date_time(now, False)
        assert status is True

        # Set back to daytime
        daytime = datetime(now.year, now.month, now.day, 12, 0, 0)

        status = world.set_sun_position_from_date_time(daytime, False)
        assert status is True

    except NNGException as err:
        raise Exception(str(err))


def test_set_and_get_sun_intensity(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        val = 2.5
        status = world.set_sunlight_intensity(val)
        assert status is True
        get_val = world.get_sunlight_intensity()
        assert get_val == val

    except NNGException as err:
        raise Exception(str(err))


def test_set_and_get_cloud_shadow_strength(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        val = 0.5
        status = world.set_cloud_shadow_strength(val)
        assert status is True
        get_val = world.get_cloud_shadow_strength()
        assert get_val == val

    except NNGException as err:
        raise Exception(str(err))


def test_failure_response_handling(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        bad_sim_get_obj_pose_req: Dict = {
            "method": f"{world.parent_topic}/get_object_pose",
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


def test_get_set_segmentation_id(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        seg_id_orig = world.get_segmentation_id_by_name("templatecube_rounded_1", True)
        assert seg_id_orig != -1

        seg_id_new = (seg_id_orig + 1) % 256
        status = world.set_segmentation_id_by_name(
            "templatecube_rounded.*", seg_id_new, True, True
        )
        assert status is True

        seg_id_resp = world.get_segmentation_id_by_name("templatecube_rounded_1", True)
        assert seg_id_resp == seg_id_new
        # TODO Add tests for each variation of parameters

        seg_id_map = world.get_segmentation_id_map()
        assert seg_id_map["TemplateCube_Rounded_1"] == seg_id_new

    except NNGException as err:
        raise Exception(str(err))


def test_switch_streaming_view(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        assert world.switch_streaming_view() is True

    except NNGException as err:
        raise Exception(str(err))


def test_plot_debug_markers(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        list = [1, 2, 3, 4, 5, 6]
        # ----------------------------------------------------------------------------------
        points = [[x, y, -5] for x, y in zip(list, list)]
        color_rgba = [1.0, 0.0, 0.0, 1.0]
        size = 10
        duration = 10
        is_persistent = True
        status = world.plot_debug_points(
            points, color_rgba, size, duration, is_persistent
        )
        assert status is True

        points_start = [[x, y, z] for x, y, z in zip(list, list, list)]
        points_end = [[x, y, z] for x, y, z in zip(list, list, list)]
        color_rgba = [1.0, 0.0, 1.0, 1.0]
        thickness = 3
        size = 15
        is_persistent = False
        status = world.plot_debug_arrows(
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
        status = world.plot_debug_solid_line(
            points, color_rgba, thickness, duration, is_persistent
        )
        assert status is True

        points = [[x, y, -7] for x, y in zip(list, list)]
        color_rgba = [0.0, 1.0, 0.0, 1.0]
        status = world.plot_debug_dashed_line(
            points, color_rgba, thickness, duration, is_persistent
        )
        assert status is True

        positions = [[x, y, -1] for x, y in zip(list, list)]
        strings = ["Microsoft AirSim" for i in range(len(positions))]
        scale = 1
        color_rgba = [1.0, 1.0, 1.0, 1.0]
        status = world.plot_debug_strings(
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
        status = world.plot_debug_transforms(
            poses, scale, thickness, duration, is_persistent
        )
        assert status is True

        names = ["yaw = " + str(round(yaw, 1)) for yaw in list]
        text_scale = 1
        status = world.plot_debug_transforms_with_names(
            poses, names, scale, thickness, text_scale, color_rgba, duration
        )
        assert status is True

        status = world.flush_persistent_markers()
        assert status is True
    except NNGException as err:
        raise Exception(str(err))


def test_trace_line(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        assert world.toggle_trace() is True

        assert world.set_trace_line([0.0, 0.0, 1.0, 1.0], 5.0) is True

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


def test_get_kinematics(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        kin = drone.get_ground_truth_kinematics()

        assert "time_stamp" in kin

        assert "pose" in kin
        assert "position" in kin["pose"]
        assert "orientation" in kin["pose"]

        assert "twist" in kin
        assert "linear" in kin["twist"]
        assert "angular" in kin["twist"]

        assert "accels" in kin
        assert "linear" in kin["accels"]
        assert "angular" in kin["accels"]
    except NNGException as err:
        raise Exception(str(err))


def test_set_kinematics(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        kin = drone.get_ground_truth_kinematics()
        kin["pose"]["position"]["x"] = 123.4
        assert drone.set_ground_truth_kinematics(kin) is True
        kin = drone.get_ground_truth_kinematics()
        assert kin["pose"]["position"]["x"] == pytest.approx(123.4, 0.1)

    except NNGException as err:
        raise Exception(str(err))


def test_get_ground_truth_geo_location(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        loc = drone.get_ground_truth_geo_location()

        assert "latitude" in loc
        assert "longitude" in loc
        assert "altitude" in loc
    except NNGException as err:
        raise Exception(str(err))


def test_can_arm(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        can_arm = drone.can_arm()
        assert can_arm is True

    except NNGException as err:
        raise Exception(str(err))


def test_get_estimated_geo_location(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        loc = drone.get_estimated_geo_location()

        assert "latitude" in loc
        assert "longitude" in loc
        assert "altitude" in loc
    except NNGException as err:
        raise Exception(str(err))


def test_get_ready_state(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        state = drone.get_ready_state()

        assert "ready_val" in state
        assert "ready_message" in state
    except NNGException as err:
        raise Exception(str(err))


def test_get_estimated_kinematics(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        kin = drone.get_estimated_kinematics()

        assert "time_stamp" in kin

        assert "pose" in kin
        assert "position" in kin["pose"]
        assert "orientation" in kin["pose"]

        assert "twist" in kin
        assert "linear" in kin["twist"]
        assert "angular" in kin["twist"]

        assert "accels" in kin
        assert "linear" in kin["accels"]
        assert "angular" in kin["accels"]
    except NNGException as err:
        raise Exception(str(err))


def test_landed_state(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        landed_state = drone.get_landed_state()
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


def test_get_set_pose(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_get_set_geo_pose(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

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


def test_get_images(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        # Test getting a single image type
        cur_simtime = world.get_sim_time()
        images = drone.get_images(
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
        cur_simtime = world.get_sim_time()
        images = drone.get_images(
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
        images = drone.get_images(camera_id="DownCamera", image_type_ids=[])

        # Check that an empty dict is returned
        assert not images

    except NNGException as err:
        raise Exception(str(err))


def test_set_camera_pose(client):
    world = World(client, "scene_test_drone.jsonc", 1)
    drone = Drone(client, world, "Drone1")

    trans = Vector3({"x": 50, "y": 60, "z": 70})
    rot = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
    pose = Pose(
        {
            "translation": trans,
            "rotation": rot,
            "frame_id": "DEFAULT_ID",
        }
    )

    assert drone.set_camera_pose("DownCamera", pose) is True


def test_set_camera_focal_length(client):
    world = World(client, "scene_test_drone.jsonc", 1)
    drone = Drone(client, world, "Drone1")

    assert drone.set_focal_length("DownCamera", ImageType.SCENE, 15.0) is True


def test_set_field_of_view(client):
    world = World(client, "scene_test_drone.jsonc", 1)
    drone = Drone(client, world, "Drone1")

    assert drone.set_field_of_view("DownCamera", ImageType.SCENE, 1.0) is True


def test_create_voxel_grid(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

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

        occupancy_grid = world.create_voxel_grid(
            center_pos, x_size, y_size, z_size, resolution
        )

        check = all(element == occupancy_grid[0] for element in occupancy_grid)

        assert len(occupancy_grid) != 0
        assert check == False  # Make sure the entire array is not either True or False

    except NNGException as err:
        raise Exception(str(err))


def test_get_bbox_3d(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        object_id = "Cone_5"
        bbox_data = world.get_3d_bounding_box(object_id, BoxAlignment.WORLD_AXIS)
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


def test_get_unexisting_bbox_3d(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

        object_id = "UnexistingObject"
        bbox_data = world.get_3d_bounding_box(object_id, BoxAlignment.WORLD_AXIS)
        print(f"bbox_data:{bbox_data}")
        assert bbox_data == {}

    except NNGException as err:
        raise Exception(str(err))


def test_get_bbox_3d_spawned(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 0)

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
        world.spawn_object(object_name, asset_name, pose, scale, enable_physics)

        # Get world-aligned bbox
        bbox_world = world.get_3d_bounding_box(object_name, BoxAlignment.WORLD_AXIS)
        print(bbox_world)
        assert Vector3(bbox_world["center"]) == translation
        assert bbox_world["quaternion"] == {"w": 1, "x": 0.0, "y": 0, "z": 0}

        # Get object-aligned bbox
        bbox_obj = world.get_3d_bounding_box(object_name, BoxAlignment.OBJECT_ORIENTED)
        print(bbox_obj)
        assert Vector3(bbox_obj["center"]) == translation
        assert bbox_obj["quaternion"] != bbox_world["quaternion"]
        size_list = Vector3(bbox_obj["size"]).to_list()
        for i in range(0, 3):
            assert abs(size_list[i] - scale[i]) < 1e-4

        # cleanup
        world.destroy_object(object_name)

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


def test_request_control_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        asyncio.run(request_control_async(drone))

    except NNGException as err:
        raise Exception(str(err))


async def set_mission_mode_async(drone):
    set_mode = await drone.set_mission_mode_async()
    await set_mode


def test_set_mission_mode_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        asyncio.run(set_mission_mode_async(drone))

    except NNGException as err:
        raise Exception(str(err))


async def set_vtol_mode_async(drone):
    set_mode = await drone.set_vtol_mode_async(Drone.VTOLMode.FixedWing)
    await set_mode


def test_set_vtol_mode_async(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")

        asyncio.run(set_vtol_mode_async(drone))

    except NNGException as err:
        raise Exception(str(err))


def test_set_external_force(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)
        drone = Drone(client, world, "Drone1")
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


def test_get_surface_elevation_at_point(client):
    try:
        world = World(client, "scene_test_drone.jsonc", 1)

        assert world.get_surface_elevation_at_point(0, 0) == pytest.approx(2.8, 0.1)
        assert world.get_surface_elevation_at_point(-10, 5) == pytest.approx(1.0)
        assert world.get_surface_elevation_at_point(21, -35) == pytest.approx(21)
        assert world.get_surface_elevation_at_point(71, -64) == pytest.approx(26)
    except NNGException as err:
        raise Exception(str(err))

@pytest.mark.usefixtures("set_trajectories")
def test_car1_trajectory(client, world, set_trajectories):
    traj_manager = set_trajectories
    time.sleep(1) # Wait for the actors to initialize
    client.subscribe("/Sim/SceneBasicDrone/env_actors/car1/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "car1", msg, traj_manager))
    time.sleep(traj_manager.get_trajectory("car1")['total_duration'] + 2)

@pytest.mark.usefixtures("set_trajectories")
def test_car2_trajectory_with_offsets(client, world, set_trajectories):
    traj_manager = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/car2/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "car2", msg, traj_manager))
    time.sleep(traj_manager.get_trajectory("car2")['total_duration'] + 2)

@pytest.mark.usefixtures("set_trajectories")
def test_pedestrian1_trajectory(client, world, set_trajectories):
    traj_manager = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/pedestrian1/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "pedestrian1", msg, traj_manager))
    time.sleep(traj_manager.get_trajectory("pedestrian1")['total_duration'] + 2)

@pytest.mark.usefixtures("set_trajectories")
def test_pedestrian2_trajectory_with_offsets(client, world, set_trajectories):
    traj_manager = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/pedestrian2/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "pedestrian2", msg, traj_manager))
    time.sleep(traj_manager.get_trajectory("pedestrian2")['total_duration'])

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
