import pytest
import random
import time
import uuid
import asyncio
from projectairsim import ProjectAirSimClient, World, EnvActor

PI = 3.14159

# Position tolerance for validation
POSITION_TOLERANCE = 0.1
# Time tolerance for validation
TIME_TOLERANCE = 0.02

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

    # Verify the actor's position at a given time against the expected trajectory
    def verify_position(self, current_time, actor_name, msg):
        # Retrieve the stored trajectory data for the given actor
        trajectory = self.trajectories.get(actor_name)
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

@pytest.fixture(scope="module")
def client():
    # Set up the AirSim client and disconnect after the tests
    client = ProjectAirSimClient()
    client.connect()
    yield client
    client.disconnect()

@pytest.fixture(scope="module")
def world(client):
    # Set up the simulation world
    world = World(client, "scene_test_env_actors_grounded.jsonc", delay_after_load_sec=2)
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

def set_env_car_traj(actor, world, x_offset=0, y_offset=0, time_offset=0):
    # Set up the trajectory and get the initial time
    start_time = world.get_sim_time() / 1e9
    time_sec = [start_time, start_time + 2]
    pose_x = [100, 105]
    pose_y = [100, 105]
    pose_z = [-2, -2]
    pose_yaw = [0, PI/2]
    pose_roll = [0] * len(time_sec)
    pose_pitch = [0] * len(time_sec)

    # Import the trajectory into the environment
    unique_id = str(uuid.uuid4())  # Generates an unique ID for the trajectory
    traj_name = f"traj_with_offset_{unique_id}" if (x_offset or y_offset or time_offset) else f"traj_{unique_id}"
    world.import_ned_trajectory(traj_name, time_sec, pose_x, pose_y, pose_z, pose_roll, pose_pitch, pose_yaw)

    # Assign the trajectory to the actor with the specified offsets
    actor.set_trajectory(traj_name, to_loop=False, time_offset=time_offset, x_offset=x_offset, y_offset=y_offset)

    time.sleep(1)  # Wait to process the trajectory

    # Store the start and end positions
    start_position = {'x': pose_x[0] + x_offset, 'y': pose_y[0] + y_offset}
    end_position = {'x': pose_x[-1] + x_offset, 'y': pose_y[-1] + y_offset}
    total_duration = time_sec[-1] - time_sec[0] + time_offset

    return start_position, end_position, start_time, total_duration

@pytest.fixture(scope="module")
def set_trajectories(env_actor1, env_actor2, env_actor3, env_actor4, world):
    # Initialize the trajectory manager to store and manage trajectory data
    traj_manager = TrajectoryManager()

    # Set up trajectories for car1 and pedestrian1 (without offset)
    start_pos1, end_pos1, start_time1, total_duration1 = set_env_car_traj(env_actor1, world)
    start_pos3, end_pos3, start_time3, total_duration3 = set_env_car_traj(env_actor3, world, 5, 5)

    traj_manager.set_trajectory_("car1", start_time1, start_pos1, end_pos1, total_duration1)
    traj_manager.set_trajectory_("pedestrian1", start_time3, start_pos3, end_pos3, total_duration3)

    # Set up trajectories for car2 and pedestrian2 (with offset)
    X_OFFSET = random.uniform(-5, 5)
    Y_OFFSET = random.uniform(-5, 5)
    TIME_OFFSET = random.uniform(1, 3)

    start_pos2, end_pos2, start_time2, total_duration2 = set_env_car_traj(env_actor2, world, X_OFFSET, Y_OFFSET, TIME_OFFSET)
    start_pos4, end_pos4, start_time4, total_duration4 = set_env_car_traj(env_actor4, world, X_OFFSET+5, Y_OFFSET+5, TIME_OFFSET)

    traj_manager.set_trajectory_("car2", start_time2, start_pos2, end_pos2, total_duration2)
    traj_manager.set_trajectory_("pedestrian2", start_time4, start_pos4, end_pos4, total_duration4)

    return traj_manager

def kinematics_callback(world, actor_name, msg, traj_manager):
    # Handle the kinematics callback and verify the actor's position
    current_time = world.get_sim_time() / 1e9
    if not traj_manager.verify_position(current_time, actor_name, msg):
        print(f"Verification failed for {actor_name} at time {current_time}")

@pytest.mark.usefixtures("set_trajectories")
@pytest.mark.asyncio
async def test_car1_trajectory(client, world, set_trajectories):
    traj_manager1 = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/car1/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "car1", msg, traj_manager1))
    await asyncio.sleep(traj_manager1.trajectories["car1"]['total_duration'])

@pytest.mark.usefixtures("set_trajectories")
@pytest.mark.asyncio
async def test_car2_trajectory_with_offsets(client, world, set_trajectories):
    traj_manager = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/car2/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "car2", msg, traj_manager))
    await asyncio.sleep(traj_manager.trajectories["car2"]['total_duration'])

@pytest.mark.usefixtures("set_trajectories")
@pytest.mark.asyncio
async def test_pedestrian1_trajectory(client, world, set_trajectories):
    traj_manager = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/pedestrian1/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "pedestrian1", msg, traj_manager))
    await asyncio.sleep(traj_manager.trajectories["pedestrian1"]['total_duration'])

@pytest.mark.usefixtures("set_trajectories")
@pytest.mark.asyncio
async def test_pedestrian2_trajectory_with_offsets(client, world, set_trajectories):
    traj_manager = set_trajectories
    client.subscribe("/Sim/SceneBasicDrone/env_actors/pedestrian2/actual_kinematics",
                     lambda topic, msg: kinematics_callback(world, "pedestrian2", msg, traj_manager))
    await asyncio.sleep(traj_manager.trajectories["pedestrian2"]['total_duration'])