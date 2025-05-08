"""
Copyright (C) Microsoft Corporation. All rights reserved.
OpenAI Gym environment wrapper for ProjectAirSim Environments
"""

from typing import Any, Dict, Tuple

import gym

from projectairsim import ProjectAirSimClient, World


class ProjectAirSimEnv(gym.Env):
    def __init__(
        self,
        sim_config_fname: str = "scene_basic_drone.jsonc",
    ):
        """ProjectAirSim sim environment interface that is OpenAI Gym compatible

        Args:
            sim_config_fname (str, optional): Sim config file name. Defaults to
             "scene_basic_drone.jsonc".
        """
        super().__init__()
        self.sim_config_fname = sim_config_fname
        self.client = ProjectAirSimClient()
        self.client.connect()
        self.world = World(self.client, self.sim_config_fname)

        self.done = False

    def reset(self):
        raise NotImplementedError

    def step(self, action: Any) -> Tuple[Any, float, bool, Dict]:
        """Perform one step in the ProjectAirSim env

        Args:
            action (Any): A valid action from the Action Space of the env

        Raises:
            NotImplementedError: [description]

        Returns:
            Tuple[Any, float, bool, Dict]: (observation, reward, done, info)
        """
        raise NotImplementedError

    def close(self):
        self.client.disconnect()
