"""
Copyright (C) Microsoft Corporation. All rights reserved.
ProjectAirSim:: Autonomy:: Gym Environment: ProjectAirSimVisualDroneLanding RL Environment for
Aerial detect-avoid using Camera observations
"""
import os
import sys
from typing import Any, Dict, List, Tuple
import asyncio

import cv2
import numpy as np
import gym.spaces as spaces
from itertools import permutations
import time

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))
from projectairsim import Drone  # noqa E402
from projectairsim.autonomy.gym_envs.gym_env import ProjectAirSimEnv  # noqa E402
from projectairsim.utils import (
    projectairsim_log,
    quaternion_to_rpy,
)  # noqa E402
from projectairsim.types import Pose, Vector3, Quaternion

PROFILING: bool = False  # TODO: Make this an env var
if PROFILING:
    import cProfile
    import pstats
    import io

ENV_CONFIG: Dict = {
    # config_fname: Path to Sim config file under sim_config dir
    "config_fname": "blocks_visual_detect_avoid.jsonc",
    # dt: Delta/step time in seconds between commanding actions, needs to be longer
    # than the minimum command duration of the flight controller (50 Hz, 20 ms)
    "dt": 0.1,
    "step_limit": 2000,  # Time budget: step_limit * dt seconds
    "goal_tolerance": 0.5,  # L1 distance from goal pose
    "randomize_obstacle_poses": False,  # Enable/Disable randomization of obstacle poses
    "num_obstacles": 3,  # Number of obstacles in the env
    "arena_config": {  # Arena ground plane scene coordinates in meters
        "top_left": {
            "x_m": 145,
            "y_m": 350,
            "z_m": -1.1,
        },
        "center": {"x_m": 96, "y_m": 398, "z_m": -1.1},
        "extent_x_m": 98,
        "extent_y_m": 96,
        "extent_z_m": 100,  # Roof height (excluding antenna) of Building1 with scale:0.5
    },
    "goals": {
        "v_vertex": {"x": 75.3, "y": 399.4, "z": 1.1},
        "v_apex_left": {"x": 136.5, "y": 370.7, "z": 1.1},
        "v_apex_right": {"x": 133.8, "y": 422.4, "z": 1.1},
    },
    "obstacles_info": {
        "PowerTower": {
            "size": {  # Size of asset at default scale=1
                "length_m": 9.6,
                "breadth_m": 22,
                "height_m": 38,
            }
        },
        "Building": {"size": {"length_m": 62.0, "breadth_m": 62.0, "height_m": 242}},
    },
    "obstacles_config": {
        "PowerTower1": {
            "type": "PowerTower",  # Key from obstacles_info
            "pose": {  # Default pose for reference
                "translation": Vector3({"x": 64.0, "y": 372.0, "z": -1.0}),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
            },
            "scale": [1, 1, 1],
            "enable_physics": False,
        },
        "PowerTower2": {
            "type": "PowerTower",  # Key from obstacles_info
            "pose": {  # Default pose for reference
                "translation": Vector3({"x": 65.2, "y": 420.0, "z": -1.0}),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
            },
            "scale": [1, 1, 1],
            "enable_physics": False,
        },
        "Building1": {
            "type": "Building",
            "pose": {  # Default pose for reference
                "translation": Vector3({"x": 105.0, "y": 400.0, "z": -1.0}),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
            },
            "scale": [0.5, 0.5, 0.5],
            "enable_physics": False,
        },
    },
    "obstacle_poses": {
        "front-left": {
            "translation": Vector3({"x": 64.0, "y": 372.0, "z": -1.0}),
            "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
        },
        "front-right": {
            "translation": Vector3({"x": 65.2, "y": 420.0, "z": -1.0}),
            "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
        },
        "back": {
            "translation": Vector3({"x": 105.0, "y": 400.0, "z": -1.0}),
            "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
        },
    },
}


def get_image_data(image_msg):
    """Get decoded image data array from image messages

    Args:
        img_msg (): [description]

    Returns:
        [type]: [description]
    """
    if image_msg is not None:
        nparr = np.frombuffer(image_msg["data"], dtype="uint8")
        img_np = np.reshape(nparr, [image_msg["height"], image_msg["width"], 3])
        return img_np
    else:
        raise ValueError("image_msg is None")


class ProjectAirSimVisualDetectAvoidEnv(ProjectAirSimEnv):
    def __init__(self, sim_config=ENV_CONFIG):
        """Drone Detect and Avoid environment with obstacles (Buildings, poles, trees).
        Observation{
          rgb_image: 3 x image_width x image_height
        },


        Args:
            sim_config (ENV_CONFIG): Environment config
        """
        self.env_config = sim_config
        self.sim_config_fname = self.env_config.get(
            "config_fname", "scene_bonsai_detect_avoid.jsonc"
        )
        super().__init__(self.sim_config_fname)
        self.dt = self.env_config.get("dt", 0.100)

        self.ego_pose_shape = (4,)  # x_world, y_world, z_world, yaw_world
        self.obstacle_bbox_shape: tuple = (6,)  # x_world, y_world, z_world, l, b, h
        self.obstacles_max: int = len(self.env_config.get("obstacles_config", 0))  # 3
        self.num_obstacles: int = self.env_config.get("num_obstacles")
        assert (
            self.num_obstacles <= self.obstacles_max
        ), f"Can't spawn {self.num_obstacles} obstacles. Max:{self.obstacles_max}"
        self.action_shape = (3,)  # V_north, V_east, V_down
        # TODO: Constrain bounds based on the scene extents
        self.observation_space: spaces.Box = spaces.Box(
            low=0.0,
            high=255.0,
            shape=(64, 64, 3),  # TODO: Read-from or sync with robot config
        )
        self.action_space = spaces.Box(-1.0, 1.0, shape=self.action_shape)
        self.goals = self.env_config.get("goals")
        self.goal_ids: List = list(self.goals.keys())  # List of Goal IDs
        # Used in reward calculations
        self.arena_config = self.env_config.get("arena_config")
        self.arena_norm_factor = np.sqrt(
            self.arena_config.get("extent_x_m") ** 2
            + self.arena_config.get("extent_y_m") ** 2
            + self.arena_config.get("extent_z_m") ** 2
        )
        self.annotation_msg = dict()
        self.goal_tolerance = self.env_config.get("goal_tolerance")
        self.img_msg = None
        self.step_num = 0
        self.obstacle_poses: Dict = self.env_config.get("obstacle_poses")
        self.available_obstacles: Dict = self.env_config.get("obstacles_config")
        # Use only self.num_obstacles number of obstacles
        self.obstacle_ids = list(self.available_obstacles.keys())[: self.num_obstacles]
        self.obstacles_config = {
            obs_id: self.available_obstacles[obs_id] for obs_id in self.obstacle_ids
        }

        self.randomize_obstacle_poses = self.env_config.get(
            "randomize_obstacle_poses", False
        )
        if self.randomize_obstacle_poses and self.num_obstacles > 1:
            self.randomized_obstacle_pose_ids = list(
                permutations(self.obstacle_poses.keys())
            )
        self.metadata = {"render.modes": ["human"]}  # Metadata for rendering

    def reset(self):
        self.done = False
        self.step_num = 0
        self.actor_pose = None
        self.state = None
        self.collision_info = None

        self.world.load_scene(self.sim_config_fname, delay_after_load_sec=0)
        self.actor = Drone(self.client, self.world, "RLDrone")
        # Randomize goals on reset
        self.goal_index: int = int(np.random.choice(len(self.goal_ids), 1))
        self.goal_id: str = self.goal_ids[self.goal_index]
        self.goal_pose = self.goals[self.goal_id]

        # Spawn obstacles

        self.spawn_obstacles(self.world, self.obstacles_config)
        # Subscribe to Actor's ego pose
        self.client.subscribe(
            self.actor.robot_info["actual_pose"],
            lambda _, pose: self.callback_pose(pose),
        )
        # Subscribe to images for visual/debugging
        self.client.subscribe(
            self.actor.sensors["ForwardViewCamera"]["scene_camera"],
            self.callback_camera,
        )
        # Subscribe to Obstacle bounding box annotations
        self.client.subscribe(
            f"{self.actor.sensors_topic}/ForwardViewCamera/image_annotation",
            self.callback_annotation,
        )
        # Subscribe to collision info
        self.client.subscribe(
            self.actor.robot_info["collision_info"],
            lambda _, collision_info: self.callback_collision(collision_info),
        )

        self.actor.enable_api_control()
        self.actor.arm()
        # Advance Sim by a dt step to get initial observation
        self.world.continue_for_sim_time(self.dt * 1e9, wait_until_complete=False)
        # Wait until an initial image observation is received
        while self.img_msg is None:
            time.sleep(self.dt)
        state = self.get_state()
        obs = state["rgb_image"]
        return obs

    def step(self, action):
        loop = asyncio.get_event_loop()
        step_coro = self.step_async(action)
        result = loop.run_until_complete(step_coro)
        return result

    async def step_async(self, action):
        if PROFILING:
            start_t = time.time()
            profile = cProfile.Profile()
            profile.enable()
        if isinstance(action, np.ndarray):
            action = action.tolist()
        # Action:{Vx: float, Vy: float, Vz: float}
        elif isinstance(action, Dict):
            action = [action["Vx"], action["Vy"], action["Vz"]]
        action = action
        v_north = action[0]
        v_east = action[1]
        v_down = action[2]

        # Latch the command for the duration of self.dt
        time_float_precision_offset = 1e-3  # TODO change MoveBy* API to take int nanos
        move_task = await self.actor.move_by_velocity_async(
            v_north, v_east, v_down, duration=(self.dt - time_float_precision_offset)
        )

        # Advance the sim by self.dt
        self.world.continue_for_sim_time(self.dt * 1e9, wait_until_complete=False)
        # Wait for async move task to complete its response
        # time.sleep(self.dt)
        try:
            await asyncio.wait_for(move_task, self.dt)
        except:
            self.actor.CancelLastTask()

        next_state = self.get_state()
        # If actor pose is at the Goal (with a tolerance), set done = True
        if (
            np.linalg.norm(next_state.get("displacement_to_goal"), ord=1)
            <= self.goal_tolerance
        ):
            self.done = True
            print(f"GOAL REACHED")
            reward = self.get_reward()
        # If actor pose is  outside the arena, set done = True, reward = -arena_norm
        elif self.is_outside_arena(next_state.get("pose")):
            self.done = True
            reward = -self.arena_norm_factor
            print(f"OUT OF ARENA")
        # Episode time budget TODO: Update as per task/goals
        elif self.step_num >= self.env_config.get("step_limit"):
            self.done = True
            reward = self.get_reward()
            print(f"EPISODE TIMEOUT")
        else:
            reward = self.get_reward()
        done = self.done
        self.step_num += 1
        info = {"status": "Running"}
        if PROFILING:
            end_t = time.time()
            profile.disable()
            stream = io.StringIO()
            ps = pstats.Stats(profile, stream=stream).sort_stats("tottime")
            ps.print_stats()
            with open("step-profile.txt", "w+") as f:
                f.write(stream.getvalue())
            info["step_time"] = end_t - start_t
            print(f"STEP_TIME_S:{end_t - start_t}")
        next_obs = next_state["rgb_image"]
        return (next_obs, reward, done, info)

    def render(self, mode="human", close=False):
        """Print the observation vector

        Args:
            mode (str, optional): Defaults to 'human'.
            close (bool, optional): Defaults to False.
        """
        if self.img_msg is not None and mode == "human":
            self.display_debug_info(self.state, self.annotation_msg)

    def get_state(self):
        # Get pose of the Robot/Drone
        if self.actor_pose is None:
            self.actor_pose = self.get_vec_from_pose(
                self.world.get_object_pose(self.actor.name)
            )
        (actor_x, actor_y, actor_z, _, _, actor_yaw) = self.actor_pose

        # Get the bboxes of obstacles
        obstacles_bboxes: Dict = self.get_obstacles_bboxes(self.annotation_msg)
        drone_pose: List = [actor_x, actor_y, actor_z, actor_yaw]
        # Populate the observation for the Agent
        self.state = {
            "pose": drone_pose,
            "rgb_image": get_image_data(self.img_msg),
            "displacement_to_goal": [
                actor_x - self.goal_pose["x"],
                actor_y - self.goal_pose["y"],
                actor_z - self.goal_pose["z"],
            ],
        }
        # print(f"Observation: {self.observation}")
        return self.state

    def get_l2_norm(self, position1, position2):
        """Return L2/Euclidean Vector norm between `position1` and `position2`"""
        l2_distance = np.linalg.norm(
            np.array(position1[:3]) - np.array(position2[:3]), ord=2
        )
        return l2_distance

    def get_reward(self):
        """Return reward based on L2 distance between goal and drone's current pose"""
        goal_pose = [self.goal_pose["x"], self.goal_pose["y"], self.goal_pose["z"]]
        reward = (
            self.arena_norm_factor - self.get_l2_norm(goal_pose, self.actor_pose)
        ) / self.arena_norm_factor
        return reward

    def callback_pose(self, pose):
        (
            actor_x,
            actor_y,
            actor_z,
            actor_roll,
            actor_pitch,
            actor_yaw,
        ) = self.get_vec_from_pose(pose)
        self.actor_pose = [
            actor_x,
            actor_y,
            actor_z,
            actor_roll,
            actor_pitch,
            actor_yaw,
        ]

    def callback_annotation(self, topic, annotation_msg):
        self.annotation_msg = annotation_msg

    def callback_camera(self, topic, image_msg):
        self.img_msg = image_msg

    def get_vec_from_pose(self, pose: Dict[str, Any]):
        """Get [x, y, z, roll, pitch, yaw] from Pose dict"""

        if "position" in pose:
            position = pose.get("position")
        else:
            position = pose.get("translation")

        x = position.get("x", 0.0)
        y = position.get("y", 0.0)
        z = position.get("z", 0.0)
        if "rotation" in pose:
            rotation = pose.get("rotation")
        else:
            rotation = pose.get("orientation")
        rot_w = rotation.get("w", 1.0)
        rot_x = rotation.get("x", 0.0)
        rot_y = rotation.get("y", 0.0)
        rot_z = rotation.get("z", 0.0)
        roll, pitch, yaw = quaternion_to_rpy(rot_w, rot_x, rot_y, rot_z)

        return (x, y, z, roll, pitch, yaw)

    def callback_collision(self, collision_info):
        self.collision_info = collision_info
        self.done = True
        print("COLLISION")
        print(f"collision_info:{collision_info}")

    def spawn_obstacles(self, world, obstacles_config: Dict):
        if self.randomize_obstacle_poses:
            # Randomize obstacle locations
            randomized_pose_index: int = int(
                np.random.choice(len(self.randomized_obstacle_pose_ids), 1)
            )
            randomized_pose_ids: Tuple = self.randomized_obstacle_pose_ids[
                randomized_pose_index
            ]
            obstacle_pose_ids = randomized_pose_ids
        else:
            # Default obstacle locations
            obstacle_pose_ids = tuple(self.obstacle_poses.keys())

        for index, obstacle_id in enumerate(obstacles_config):
            obstacle = obstacles_config.get(obstacle_id)
            world.spawn_object(
                obstacle_id,
                obstacle.get("type"),
                Pose(self.obstacle_poses.get(obstacle_pose_ids[index])),
                obstacle.get("scale"),
                obstacle.get("enable_physics"),
            )

    def flatten_bbox3d(_, annotation):
        bbox3d = annotation.get("bbox3d", None)
        if bbox3d is not None:
            return [
                bbox3d["center"]["x"],
                bbox3d["center"]["y"],
                bbox3d["center"]["z"],
                bbox3d["size"]["x"],
                bbox3d["size"]["y"],
                bbox3d["size"]["z"],
            ]

    def get_obstacles_bboxes(self, annotation_msg):
        if annotation_msg.get("annotations"):
            obstacles_bboxes = {
                annotation["object_id"]: self.flatten_bbox3d(annotation)
                for annotation in annotation_msg.get("annotations")
            }
            return obstacles_bboxes
        else:
            # Return null obstacles with size=-1 if no obstacles found
            null_obstacle = [-1 for _ in range(self.obstacle_bbox_shape[0])]
            return {f"null{o}": null_obstacle for o in range(self.num_obstacles)}

    def is_outside_arena(self, pose: List):
        x, y, z, _ = pose
        # Axis aligned arena
        arena_max_x = self.arena_config.get("top_left")["x_m"]
        arena_min_y = self.arena_config.get("top_left")["y_m"]
        arena_min_z = self.arena_config.get("top_left")["z_m"]
        arena_min_x = arena_max_x - self.arena_config.get("extent_x_m")
        arena_max_y = arena_min_y + self.arena_config.get("extent_y_m")
        arena_max_z = -(abs(arena_min_z) + self.arena_config.get("extent_z_m"))
        if (
            x <= arena_min_x
            or x >= arena_max_x
            or y <= arena_min_y
            or y >= arena_max_y
            or z >= arena_min_z  # NED; z -ve when up
            or z <= arena_max_z  # NED; z -ve when up
        ):
            return True
        return False

    def display_debug_info(
        self, state, annotation_msg, win_name="ProjectAirSimDetectAvoid"
    ):
        if state["rgb_image"] is not None:
            img_np = state["rgb_image"]
            ## Display 2D Bboxes for debugging
            for annotation in annotation_msg["annotations"]:
                bbox_center = annotation["bbox2d"]["center"]
                bbox_size = annotation["bbox2d"]["size"]
                v1 = (
                    int(bbox_center["x"] - (bbox_size["x"] / 2.0)),
                    int(bbox_center["y"] - (bbox_size["y"] / 2.0)),
                )
                v2 = (
                    int(bbox_center["x"] + (bbox_size["x"] / 2.0)),
                    int(bbox_center["y"] + (bbox_size["y"] / 2.0)),
                )
                cv2.rectangle(img_np, v1, v2, (0, 255, 0), 3)
            cv2.imshow(win_name, img_np)
            cv2.waitKey(15)
            global key
            key = cv2.waitKeyEx(20)


async def main():
    # Requires Sim server to be running already
    env = ProjectAirSimVisualDetectAvoidEnv()
    num_episodes = 20
    num_steps_per_episode = 500
    for episode in range(num_episodes):
        obs = env.reset()
        for step in range(num_steps_per_episode):
            # action = env.action_space.sample()
            action = np.array([0.5, 0, -0.50])  # Move forward & up
            next_obs, reward, done, info = await env.step(action)
            print(f"Episode#:{episode} step#:{step} reward:{reward} done:{done}")
            env.render()
            if done:
                break
    env.close()


if __name__ == "__main__":
    asyncio.run(main())
