"""
This file contains the ProjectAirSimDroneLanding simulator with a perception brain
that can land/control the drone using actions from the Bonsai Platform
"""

# pyright: strict

from jinja2 import Template
import logging
import os
import sys
import time
from typing import Any, Dict

from bonsai3 import ServiceConfig, SimulatorInterface

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))

from projectairsim.gym_envs.gym_simulator3 import GymSimulator3  # noqa: E402

log = logging.getLogger("gym_simulator3")
log.setLevel(logging.DEBUG)


SLEEP_TIME = 0.0  # in seconds


class ProjectAirSimDroneLanding(GymSimulator3):
    # Environment name, from openai-gym
    # TODO: Make the Env configurable on the commandline?
    # environment_name = "ProjectAirSimDroneLanding-v0"
    environment_name = "ProjectAirSimDroneLandingWithPerception-v0"

    # simulator name from Inkling
    # Example Inkling:
    #   curriculum landing_curriculum
    #       train landing
    #       with simulator dronelanding_simulator
    #       ....
    simulator_name = "drone_landing"

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
            "predicted_actor_x": observation[0],
            "predicted_actor_y": observation[1],
            "predicted_actor_z": observation[2],
            "predicted_actor_yaw": observation[3],
            "true_actor_x": observation[4],
            "true_actor_y": observation[5],
            "true_actor_z": observation[6],
            "true_actor_roll": observation[7],
            "true_actor_pitch": observation[8],
            "true_actor_yaw": observation[9],
            "landing_pad_x": observation[10],
            "landing_pad_y": observation[11],
            "landing_pad_z": observation[12],
            "landing_pad_roll": observation[13],
            "landing_pad_pitch": observation[14],
            "landing_pad_yaw": observation[15],
            "diff_x": observation[16],
            "diff_y": observation[17],
            "diff_z": observation[18],
            "diff_yaw": observation[19],
        }
        time.sleep(SLEEP_TIME)
        print("state", state)
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

    def get_interface(self) -> SimulatorInterface:
        interface_file_path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), "drone_interface.json"
        )

        # load the template
        try:
            with open(interface_file_path, "r") as file:
                template_str = file.read()
        except:
            log.info(
                "Failed to load interface template file: {}".format(interface_file_path)
            )
            raise

        # render the template with our constants
        template = Template(template_str)
        interface_str = template.render()

        return SimulatorInterface(
            context=self.get_simulator_context(), json_interface=interface_str
        )


if __name__ == "__main__":

    config = ServiceConfig(argv=sys.argv)
    sim = ProjectAirSimDroneLanding(config)
    sim.run_gym()
