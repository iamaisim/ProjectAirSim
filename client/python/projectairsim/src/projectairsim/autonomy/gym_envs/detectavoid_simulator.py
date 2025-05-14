"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSim:: Autonomy:: RL Gym Environments: Bonsai-SD3-compatible ProjectAirSimDetectAvoid Gym env
This file contains the ProjectAirSimDetectAvoid simulator interface that
 allows training a Bonsai Brain to avoid obstacles and reach desired goals
"""

# pyright: strict

import logging
from typing import Any, Dict


from projectairsim.autonomy.gym_envs.gym_simulator3_nosdk import (
    GymSimulator3,
)  # noqa: E402

log = logging.getLogger("gym_simulator3")
log.setLevel(logging.DEBUG)


class ProjectAirSimDetectAvoid(GymSimulator3):
    def __init__(
        self, env_name: str = "ProjectAirSimDetectAvoidEnv-v0", env_config: Dict = {}
    ):
        # simulator name from Inkling
        # Example Inkling:
        #   curriculum landing_curriculum
        #       train landing
        #       with simulator dronelanding_simulator
        #       ....
        self.env_name = env_name
        self.env_config = env_config
        super().__init__(env_name=self.env_name, env_config=self.env_config)

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
            "pose": observation.get("pose"),
            "obstacles": observation.get("obstacles"),
            "distance_to_obstacles": observation.get("distance_to_obstacles"),
            "displacement_to_goal": observation.get("displacement_to_goal"),
        }
        return state

    # convert our action schema into openai gym action
    # Example Inkling:
    #   schema Action
    #       Float32 Vx,
    #       Float32 Vy,
    #       Float32 Vz
    #   end
    def action_to_gym(self, action: Dict[str, Any]):
        return {"Vx": action["Vx"], "Vy": action["Vy"], "Vz": action["Vz"]}


# if __name__ == "__main__":
#
#     config = ServiceConfig(argv=sys.argv)
#     sim = ProjectAirSimDetectAvoid(config)
#     sim.run_gym()
