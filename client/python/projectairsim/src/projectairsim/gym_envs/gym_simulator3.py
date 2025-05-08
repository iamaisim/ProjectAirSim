# pyright: reportUnusedImport=false

import gym
import argparse
from typing import Dict, Any
from bonsai3 import ServiceConfig, SimulatorInterface, SimulatorSession
from bonsai3.logger import Logger
from time import time
import projectairsim.gym_envs  # noqa: F401

log = Logger()

STATE_REWARD_KEY = "_gym_reward"
STATE_TERMINAL_KEY = "_gym_terminal"


class GymSimulator3(SimulatorSession):
    simulator_name = ""  # name of the simulation in the inkling file
    environment_name = ""  # name of the OpenAI Gym environment

    def __init__(
        self, config: ServiceConfig, iteration_limit: int = 0, skip_frame: int = 1
    ) -> None:
        super(GymSimulator3, self).__init__(config)

        # create and reset the gym environment
        self._env = gym.make(self.environment_name)
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

        # book keeping for rate status
        self.iteration_count = 0
        self.episode_count = 0
        self._log_interval = 10.0  # seconds
        self._last_status = time()

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

    def gym_episode_start(self, parameters):
        """
        called during episode_start() to return the initial observation
        after reseting the gym environment. clients can override this
        to provide additional initialization.
        """
        observation = self._env.reset()
        log.gym("start state: " + str(observation))
        return observation

    def gym_simulate(self, gym_action):
        """
        called during simulate to single step the gym environment
        and return (observation, reward, done, info).
        clients can override this method to provide additional
        reward shaping.
        """
        observation, reward, done, info = self._env.step(gym_action)
        return observation, reward, done, info

    def run_gym(self):
        """
        runs the simulation until cancelled or finished
        """
        while self.run():
            continue
        log.info("Simulator finished running")

    #
    # SDK3 SimulatorSession methods
    #

    def get_interface(self):
        face = SimulatorInterface(self.get_simulator_context())
        face.name = self.simulator_name
        face.timeout = 60
        return face

    def get_state(self):
        return self._last_state

    def episode_start(self, config: Dict[str, Any]):
        self.iteration_count = 0
        self.episode_reward = 0

        # optional configuration arguments for open-ai-gym
        if "iteration_limit" in config:
            self._iteration_limit = config["iteration_limit"]

        # initial observation
        observation = self.gym_episode_start(config)
        state = self.gym_to_state(observation)
        self._set_last_state(state, 0, False)
        return state

    def episode_step(self, action: Dict[str, Any]):

        # simulate
        gym_action = self.action_to_gym(action)
        rwd_accum = 0
        done = False
        i = 0
        observation = None

        for i in range(self._skip_frame):
            observation, reward, done, info = self.gym_simulate(gym_action)
            self.iteration_count += 1
            rwd_accum += reward

            log.gym(
                "step action: {} state: {} reward: {} done: {}".format(
                    str(gym_action), str(observation), str(reward), str(done)
                )
            )

            # episode limits
            if self._iteration_limit > 0:
                if self.iteration_count >= self._iteration_limit:
                    done = True
                    log.gym("iteration_limit reached.")
                    break

            # render if not headless
            if not self._headless:
                if "human" in self._env.metadata["render.modes"]:
                    self._env.render()

        # print a periodic status of iterations and episodes
        self._periodic_status_update()

        # calculate reward
        reward = rwd_accum / (i + 1)
        self.episode_reward += reward

        # convert state and return to the server
        state = self.gym_to_state(observation)
        self._set_last_state(state, reward, done)
        return state, reward, done

    def episode_finish(self, reason: str):
        log.info(
            "Episode {} reward is {}".format(self.episode_count, self.episode_reward)
        )
        log.gym(
            "finish episode: "
            + str(self.episode_count)
            + " reward: "
            + str(self.episode_reward)
        )
        self.episode_count += 1
        self._last_status = time()

    def halted(self):
        return False

    #
    # Internal methods
    #

    def _set_last_state(self, state: Dict[str, Any], reward: float, terminal: bool):
        self._last_state = state
        self._last_state[STATE_REWARD_KEY] = reward
        self._last_state[STATE_TERMINAL_KEY] = terminal

    def _periodic_status_update(self):
        """print a periodic status update showing iterations/sec"""
        if time() - self._last_status > self._log_interval:
            log.info(
                "Episode {} is still running, "
                "reward so far is {}".format(self.episode_count, self.episode_reward)
            )
            self._last_status = time()

    def _check_headless(self) -> bool:
        parser = argparse.ArgumentParser()
        parser.add_argument("--headless", action="store_true", default=False)
        args, unknown = parser.parse_known_args()
        headless = args.headless
        if headless:
            log.gym(
                "Running simulator headlessly, graphical "
                "environment will not be displayed."
            )
        else:
            log.gym(
                "Starting simulator with graphical evironment. "
                "Use --headless to disable."
            )
        return headless
