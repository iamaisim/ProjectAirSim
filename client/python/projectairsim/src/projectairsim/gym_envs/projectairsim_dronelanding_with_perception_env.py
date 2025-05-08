import math
import os
import sys
from typing import Any, Dict
import asyncio

import cv2
import numpy as np
from gym.spaces import Box

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))
from projectairsim import Drone  # noqa E402
from projectairsim.gym_envs.gym_env import ProjectAirSimEnv  # noqa E402
from projectairsim.image_utils import ImageDisplay
from projectairsim.utils import quaternion_to_rpy  # noqa E402

from perception_brain import DronePosePredictor


class ProjectAirSimDroneLandingWithPerceptionEnv(ProjectAirSimEnv):
    def __init__(self):
        # self.sim_config_fname = "scene_bonsai_drone_landing.jsonc"
        self.sim_config_fname = "scene_bonsai_drone_landing_sydney.jsonc"

        # Delta/step time in seconds between commanding actions, needs to be longer
        # than the minimum command duration of the flight controller (50 Hz, 20 ms)
        self.dt = 0.100

        super().__init__(self.sim_config_fname)
        self.observation_shape = (4,)  # dx, dy, dz, dyaw
        self.action_shape = (3,)  # V_north, V_east, V_down

        self.observation_space = Box(-math.inf, math.inf, shape=self.observation_shape)
        self.action_space = Box(-1.0, 1.0, shape=self.action_shape)

        # The following are not necessary if env.reset() is assured to be called
        # on episode start
        self.world.load_scene(self.sim_config_fname, delay_after_load_sec=0)

        self.actor = Drone(self.client, self.world, "BonsaiDrone")
        self.goal_pose = self.get_vec_from_pose(
            self.world.get_object_pose("droneLandingPad_2")  # "TemplateCube_Rounded_150"
        )
        self.client.subscribe(
            self.actor.robot_info["actual_pose"],
            lambda _, pose: self.pose_callback(pose),
        )
        self.client.subscribe(
            self.actor.robot_info["collision_info"],
            lambda _, collision_info: self.collision_callback(collision_info),
        )

        self.init_perception()
        self.dir_path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), "images"
        )
        self.episode_count = 0
        self.img_np = np.zeros((224, 244, 3))

    def init_perception(self):
        perc_brain_arch = "cnn"
        perc_brain_cnn_arch = "resnet18"
        # perc_trained_brain = "trained_models/sydney/cnn/best_brain_state.pth.tar"
        perc_trained_brain = "trained_models/sydney/cnn/new-checkpoint.pth.tar"
        perc_gpu = 0
        self.drone_pose_predictor = DronePosePredictor(
            perc_brain_arch, perc_brain_cnn_arch, perc_trained_brain, perc_gpu
        )

    def reset(self):
        self.episode_count += 1
        self.iteration_count = 0
        print("")
        print("NEW EPISODE", self.episode_count)
        print("self.iteration_count", self.iteration_count)

        self.done = False
        self.observation = None
        self.collision_info = None

        self.world.load_scene(self.sim_config_fname, delay_after_load_sec=0)
        print(f"Object list:{self.world.SimListSceneObjects('.*')}")

        # self.actor_pose = None
        self.goal_pose = self.get_vec_from_pose(
            self.world.get_object_pose("droneLandingPad_2")  # "TemplateCube_Rounded_150"
        )
        print("Laningpad pose is {}".format(self.goal_pose))

        self.client.subscribe(
            self.actor.sensors["DownCamera"]["scene_camera"],
            lambda _, image: self.camera_callback(image),
        )
        # image_display = ImageDisplay()
        # self.client.subscribe(
        #    self.actor.sensors["DownCamera"]["scene_camera"], lambda _, image: image_display.display(image, "RGB-Image")
        # )
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
        self.iteration_count += 1
        print("")
        print("self.iteration_count", self.iteration_count)
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
        # import pdb; pdb.set_trace()
        # Get pose of landing pad
        (
            landing_pad_x,
            landing_pad_y,
            landing_pad_z,
            landing_pad_roll,
            landing_pad_pitch,
            landing_pad_yaw,
        ) = self.goal_pose

        # if self.actor_pose is None:
        self.actor_pose = self.drone_pose_predictor.predict(self.get_rgb_image())
        (actor_x, actor_y, actor_z, actor_yaw) = self.actor_pose
        print("Predicted Actor Pose is {}".format(self.actor_pose))

        (
            ac_actor_x,
            ac_actor_y,
            ac_actor_z,
            ac_actor_roll,
            ac_actor_pitch,
            ac_actor_yaw,
        ) = self.actual_actor_pose
        print("Actual Actor Pose is {}".format((self.actual_actor_pose)))
        # Obs is pose of drone w.r.t landing-pad. That is, landing-pad is centered
        # at (0,0,0). This is to be consistent with the training target data used
        # in perception model training
        # self.observation = [
        #    ac_actor_x - landing_pad_x,
        #    ac_actor_y - landing_pad_y,
        #    ac_actor_z - landing_pad_z,
        #    ac_actor_yaw - landing_pad_yaw,
        # ]
        self.observation = [
            actor_x,
            actor_y,
            actor_z,
            actor_yaw,
            ac_actor_x,
            ac_actor_y,
            ac_actor_z,
            ac_actor_roll,
            ac_actor_pitch,
            ac_actor_yaw,
            landing_pad_x,
            landing_pad_y,
            landing_pad_z,
            landing_pad_roll,
            landing_pad_pitch,
            landing_pad_yaw,
            actor_x - landing_pad_x,
            actor_y - landing_pad_y,
            actor_z - landing_pad_z,
            actor_yaw - landing_pad_yaw,
        ]

        return self.observation

    def get_l2_distance(self, position1, position2):
        """Return L2/Euclidean distance between `position1` and `position2`"""
        return np.linalg.norm(np.array(position1) - np.array(position2))

    def get_reward(self):
        """Return reward based on L2 distance between goal and drone's current pose"""
        # distance of drone and landinpad based on actual drone pose from sim
        dist = self.get_l2_distance(self.goal_pose, self.actual_actor_pose)
        # distance of drone and landinpad based on predicted drone pose from perception model
        # dist = self.get_l2_distance(self.goal_pose, self.actor_pose)
        print("Dist is {}".format(dist))
        close_enough = 1.23
        reward = (close_enough / dist) * 10
        print("Reward is {}".format(reward))
        return reward

    def camera_callback(self, image):
        if image is not None:
            nparr = np.frombuffer(image["data"], dtype="uint8")  # noqa F481
            self.img_np = np.reshape(nparr, [image["height"], image["width"], 3])

    def get_rgb_image(self):
        img_name = (
            "syndey_ep_"
            + str(self.episode_count)
            + "_iter_"
            + str(self.iteration_count)
            + ".png"
        )
        img_path = os.path.join(self.dir_path, img_name)
        # cv2.imwrite(img_path, self.img_np)
        print("rgb images {} max is {}".format(img_name, self.img_np.max()))
        return self.img_np

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
        # return (10.0, 91.5, -1.0, roll, pitch, yaw)

    def collision_callback(self, collision_info):
        self.collision_info = collision_info
        self.done = True
        print(f"collision_info:{collision_info}")

    def pose_callback(self, pose):
        (
            actor_x,
            actor_y,
            actor_z,
            actor_roll,
            actor_pitch,
            actor_yaw,
        ) = self.get_vec_from_pose(pose)
        self.actual_actor_pose = [
            actor_x,
            actor_y,
            actor_z,
            actor_roll,
            actor_pitch,
            actor_yaw,
        ]


async def main():
    # Requires Sim server to be running already
    env = ProjectAirSimDroneLandingWithPerceptionEnv()
    num_episodes = 2
    num_steps_per_episode = 500
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
