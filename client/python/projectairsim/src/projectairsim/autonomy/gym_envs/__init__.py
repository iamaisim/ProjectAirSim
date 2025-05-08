import sys
import os
from gym.envs.registration import register


register(
    id="ProjectAirSimDetectAvoidEnv-v0",
    entry_point="projectairsim.autonomy.gym_envs.projectairsim_detectavoid_env:"
    "ProjectAirSimDetectAvoidEnv",
)

register(
    id="ProjectAirSimVisualDetectAvoidEnv-v0",
    entry_point="projectairsim.autonomy.gym_envs.projectairsim_visual_detectavoid_env:"
    "ProjectAirSimVisualDetectAvoidEnv",
    kwargs={"env_config": {}},
)
