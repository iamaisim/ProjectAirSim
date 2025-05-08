"""
This file contains the ProjectAirSimDroneLanding simulator that
 can land/control the drone using actions from the Bonsai Platform
"""

# pyright: strict

import logging
import os
import sys
import time
from typing import Any, Dict

from bonsai3 import ServiceConfig

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))

from projectairsim.gym_envs.gym_simulator3 import GymSimulator3  # noqa: E402

log = logging.getLogger("gym_simulator3")
log.setLevel(logging.DEBUG)


SLEEP_TIME = 0.0  # in seconds


class ProjectAirSimDroneLanding(GymSimulator3):
    # Environment name, from openai-gym
    environment_name = "ProjectAirSimDroneLandingSydney-v0"

    # simulator name from Inkling
    # Example Inkling:
    #   curriculum landing_curriculum
    #       train landing
    #       with simulator dronelanding_simulator
    #       ....
    simulator_name = "drone_landing_sydney"

    # convert openai gym observation to our state schema
    # Example Inkling:
    #   schema GameState
    #       Float32 x,
    #       Float32 y,
    #       Float32 z,
    #       Float32 yaw
    #   end
    def gym_to_state(self, observation: Any) -> Dict[str, Any]:
        state = {
            "landingpad_x": observation[0],
            "landingpad_y": observation[1],
            "landingpad_z": observation[2],
            "landingpad_yaw": observation[3],
        }
        time.sleep(SLEEP_TIME)
        return state

    # convert our action schema into openai gym action
    # Example Inkling:
    #   schema Action
    #       Float32 Vx,
    #       Float32 Vy,
    #       Float32 Vz
    #   end
    def action_to_gym(self, action: Dict[str, Any]):
        return [action["Vx"], action["Vy"], action["Vz"]]


if __name__ == "__main__":

    config = ServiceConfig(argv=sys.argv)
    sim = ProjectAirSimDroneLanding(config)
    sim.run_gym()
