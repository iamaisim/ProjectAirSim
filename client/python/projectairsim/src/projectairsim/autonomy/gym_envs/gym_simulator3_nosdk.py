"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSim:: Autonomy:: Gym Environment: ProjectAirSim-Bonsai connector
"""

# pyright: reportUnusedImport=false

import gym
import argparse
from typing import Dict, Any
from time import time
import projectairsim.autonomy.gym_envs  # noqa: F401

STATE_REWARD_KEY = "_gym_reward"
STATE_TERMINAL_KEY = "_gym_terminal"


class GymSimulator3:
    simulator_name = ""  # name of the simulation in the inkling file
    environment_name = ""  # name of the OpenAI Gym environment

    def __init__(
        self,
        env_name,
        iteration_limit: int = 0,
        skip_frame: int = 1,
        env_config: Dict = {},
    ) -> None:
        super(GymSimulator3, self).__init__()
        self.environment_name = env_name
        # create and reset the gym environment
        self._env = gym.envs.make(self.environment_name, env_config=env_config)
        self._env.seed(20)
        initial_observation = self._env.reset()

        # store initial gym state
        try:
            state = self.gym_to_state(initial_observation)
        except NotImplementedError as e:
            raise e
        self._set_last_state(state, 0, False)

        # optional parameters for controlling the simulation
        self._headless = self._check_headless()
        self._iteration_limit = iteration_limit  # default is no limit
        self._skip_frame = skip_frame  # default is to process every frame

    #
    # These MUST be implemented by the simulator.
    #

    def gym_to_state(self, observation: Any) -> Dict[str, Any]:
        """Convert a gym observation into an Inkling state

        Example:
            state = {'position': observation[0],
                     'velocity': observation[1],
                     'angle':    observation[2],
                     'rotation': observation[3]}
            return state

        :param observation: gym observation, see specific gym
            environment for details.
        :return A dictionary matching the Inkling state schema.
        """
        raise NotImplementedError("No gym_to_state() implementation found.")

    def action_to_gym(self, action) -> Dict[str, Any]:
        """Convert an Inkling action schema into a gym action.

        Example:
            return action['command']

        :param action: A dictionary as defined in the Inkling schema.
        :return A gym action as defined in the gym environment
        """
        raise NotImplementedError("No action_to_gym() implementation found.")

    #
    # These MAY be implemented by the simulator.
    #

    def gym_episode_start(self, env_config):
        """
        called during episode_start() to return the initial observation
        after reseting the gym environment. clients can override this
        to provide additional initialization.
        """
        observation = self._env.reset(env_config)
        print("start state: " + str(observation))
        return observation

    async def gym_simulate(self, gym_action):
        """
        called during simulate to single step the gym environment
        and return (observation, reward, done, info).
        clients can override this method to provide additional
        reward shaping.
        """
        observation, reward, done, info = await self._env.step(gym_action)
        return observation, reward, done, info

    def run_gym(self):
        """
        runs the simulation until cancelled or finished
        """
        while self.run():
            continue
        print("Simulator finished running")

    #
    # Internal methods
    #

    def _set_last_state(self, state: Dict[str, Any], reward: float, terminal: bool):
        self._last_state = state
        self._last_state[STATE_REWARD_KEY] = reward
        self._last_state[STATE_TERMINAL_KEY] = terminal

    def _check_headless(self) -> bool:
        parser = argparse.ArgumentParser()
        # Temp fix to disable rendering; TODO: Remove argparser here
        parser.add_argument("--render", action="store_true", default=False)
        args, unknown = parser.parse_known_args()
        headless = not args.render
        if headless:
            print(
                "Running simulator headlessly, graphical "
                "environment will not be displayed."
            )
        else:
            print(
                "Starting simulator with graphical evironment. "
                "Use --headless to disable."
            )
        return headless
