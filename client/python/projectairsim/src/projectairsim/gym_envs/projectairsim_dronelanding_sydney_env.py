import math
import os
import sys
from typing import Any, Dict
import asyncio

import numpy as np
from gym.spaces import Box

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))
from projectairsim import Drone  # noqa E402
from projectairsim.gym_envs.gym_env import ProjectAirSimEnv  # noqa E402
from projectairsim.utils import quaternion_to_rpy  # noqa E402


class ProjectAirSimDroneLandingSydneyEnv(ProjectAirSimEnv):
    def __init__(self):
        self.sim_config_fname = "scene_bonsai_drone_landing_sydney.jsonc"

        # Delta/step time in seconds between commanding actions, needs to be longer
        # than the minimum command duration of the flight controller (50 Hz, 20 ms)
        self.dt = 0.100

        super().__init__(self.sim_config_fname)
        self.observation_shape = (4,)  # dx, dy, dz, dyaw
        self.action_shape = (3,)  # V_north, V_east, V_down

        self.observation_space = Box(-math.inf, math.inf, shape=self.observation_shape)
        self.action_space = Box(-1.0, 1.0, shape=self.action_shape)

    def reset(self):
        self.done = False
        self.actor_pose = None
        self.observation = None
        self.collision_info = None

        self.world.load_scene(self.sim_config_fname, delay_after_load_sec=0)
        self.actor = Drone(self.client, self.world, "BonsaiDrone")

        # self.goal_pose = self.get_vec_from_pose(
        #    self.world.get_object_pose("droneLandingPad")
        # )
        self.goal_pose = [10.0, 91.5, -8.2, 0.0, 0.0, 0.0]

        self.client.subscribe(
            self.actor.sensors["DownCamera"]["scene_camera"],
            lambda _, image: self.camera_callback(image),
        )
        self.client.subscribe(
            self.actor.robot_info["actual_pose"],
            lambda _, pose: self.pose_callback(pose),
        )
        self.client.subscribe(
            self.actor.robot_info["collision_info"],
            lambda _, collision_info: self.collision_callback(collision_info),
        )

        self.actor.enable_api_control()
        self.actor.arm()

        return self.get_observation()

    async def step(self, action):
        if isinstance(action, np.ndarray):
            action = action.tolist()
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
        self.world.continue_for_sim_time(self.dt * 1e9)

        # Wait for async move task to complete its response
        await move_task

        next_obs = self.get_observation()
        reward = self.get_reward()
        # TODO: Set done if landed successfully
        done = self.done
        info = {"status": "Running"}
        return (next_obs, reward, done, info)

    def render(self, mode="human", close=False):
        """Print the observation vector

        Args:
            mode (str, optional): Defaults to 'human'.
            close (bool, optional): Defaults to False.
        """
        if self.observation is not None:
            print(f"Obs:{self.observation}")

    def get_observation(self):
        # Get pose of landing pad
        (
            landing_pad_x,
            landing_pad_y,
            landing_pad_z,
            _,
            _,
            landing_pad_yaw,
        ) = self.goal_pose
        if self.actor_pose is None:
            self.actor_pose = self.get_vec_from_pose(
                self.world.get_object_pose(self.actor.name)
            )
        (actor_x, actor_y, actor_z, _, _, actor_yaw) = self.actor_pose

        # Obs is pose of drone w.r.t landing-pad. That is, landing-pad is centered
        # at (0,0,0). This is to be consistent with the training target data used
        # in perception model training
        self.observation = [
            actor_x - landing_pad_x,
            actor_y - landing_pad_y,
            actor_z - landing_pad_z,
            actor_yaw,
        ]
        return self.observation

    def get_l2_distance(self, position1, position2):
        """Return L2/Euclidean distance between `position1` and `position2`"""
        l2_distance = math.sqrt(
            (position1[0] - position2[0]) ** 2
            + (position1[1] - position2[1]) ** 2
            + (position1[2] - position2[2]) ** 2
        )
        return l2_distance

    def get_reward(self):
        """Return reward based on L2 distance between goal and drone's current pose"""
        reward = self.get_l2_distance(self.goal_pose, self.actor_pose)
        # Alternatively, if the vector distance is the obs/state, reward is |obs|
        return reward

    def pose_callback(self, pose):
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

    def camera_callback(self, image):
        if image is not None:
            nparr = np.frombuffer(image["data"], dtype="uint8")  # noqa F481
            # TODO Get inference from the perception model and update
            # self.observation

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

    def collision_callback(self, collision_info):
        self.collision_info = collision_info
        self.done = True
        print(f"collision_info:{collision_info}")


async def main():
    # Requires Sim server to be running already
    env = ProjectAirSimDroneLandingSydneyEnv()
    num_episodes = 2
    num_steps_per_episode = 2000
    for episode in range(num_episodes):
        obs = env.reset()
        for step in range(num_steps_per_episode):
            # action = env.action_space.sample()
            action = np.array([0, 0, 4.0])  # Move down
            next_obs, reward, done, info = await env.step(action)
            print(f"Episode#:{episode} step#:{step} reward:{reward} done:{done}")
            env.render()
            if done:
                break
    env.close()


if __name__ == "__main__":
    asyncio.run(main())
