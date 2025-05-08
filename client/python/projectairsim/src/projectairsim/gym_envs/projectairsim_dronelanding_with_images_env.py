from projectairsim.gym_envs.gym_env import ProjectAirSimEnv
from projectairsim import Drone
from gym.spaces import Box
import cv2
import numpy as np
import asyncio


class ProjectAirSimDroneLandingWithImagesEnv(ProjectAirSimEnv):
    def __init__(self):
        self.sim_config_fname = "scene_bonsai_drone_landing.jsonc"

        # Delta/step time in seconds between commanding actions, needs to be longer
        # than the minimum command duration of the flight controller (50 Hz, 20 ms)
        self.dt = 0.100

        super().__init__(self.sim_config_fname)
        self.obs_width = 320  # Width of image observations
        self.obs_height = 240  # Height of image observations
        self.obs_channels = 3  # RGB ==> 3 channels
        self.action_shape = (3,)  # V_north, V_east, V_down
        self.observation_space = Box(
            0.0, 255.0, shape=(self.obs_width, self.obs_height, self.obs_channels)
        )
        self.action_space = Box(-1.0, 1.0, shape=self.action_shape)

    def reset(self):
        self.done = False
        self.observation = None
        self.collision_info = None

        self.world.load_scene(self.sim_config_fname, delay_after_load_sec=0)
        self.actor = Drone(self.client, self.world, "Drone1")

        self.client.subscribe(
            self.actor.sensors["DownCamera"]["scene_camera"],
            lambda _, image: self.camera_callback(image),
        )
        self.client.subscribe(
            self.actor.robot_info["collision_info"],
            lambda _, collision_info: self.collision_callback(collision_info),
        )

        self.actor.enable_api_control()
        self.actor.arm()

        return self.get_observation()

    async def step(self, action):
        action = action.tolist()
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
        # TODO: Define a reward function based on distance to landing target
        reward = 0.0
        # TODO: Set done if landed successfully
        done = self.done
        info = {"status": "Running"}
        return (next_obs, reward, done, info)

    def render(self, mode="human", close=False):
        """Display the camera observations

        Args:
            mode (str, optional): Defaults to 'human'.
            close (bool, optional): Defaults to False.
        """
        if self.observation is not None:
            self.display_image(self.observation)

    def get_observation(self):
        return self.observation  # Can be None if camera_callback was never called

    def camera_callback(self, image):
        if image is not None:
            nparr = np.frombuffer(image["data"], dtype="uint8")
            self.observation = np.reshape(
                nparr, [self.obs_height, self.obs_width, self.obs_channels]
            )

    def display_image(self, image, win_name: str = "Drone"):
        """Display the image using OpenCV HighGUI"""
        cv2.imshow(win_name, image)
        key = cv2.waitKey(5)
        if key == 27:  # Esc key
            cv2.destroyWindow(win_name)
            cv2.destroyAllWindows()
            print("Stopping Application. Press Ctrl+C twice to quit")
            exit()

    def collision_callback(self, collision_info):
        self.collision_info = collision_info
        self.done = True
        print(f"collision_info:{collision_info}")


async def main():
    # Requires Sim server to be running already
    env = ProjectAirSimDroneLandingWithImagesEnv()
    num_episodes = 2
    num_steps_per_episode = 2000
    for episode in range(num_episodes):
        obs = env.reset()
        for step in range(num_steps_per_episode):
            # action = env.action_space.sample()
            action = np.array([0, 0, 4.0])  # Move down
            next_obs, reward, done, info = await env.step(action)
            print(f"Episode#:{episode} step#:{step} reward:{reward} done:{done}")
            if done:
                break
    env.close()


if __name__ == "__main__":
    asyncio.run(main())
