import sys
import os
from gym.envs.registration import register

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))

register(
    id="ProjectAirSimDroneLandingWithImages-v0",
    entry_point="projectairsim.gym_envs.projectairsim_dronelanding_with_images_env:"
    "ProjectAirSimDroneLandingWithImagesEnv",
)

register(
    id="ProjectAirSimDroneLanding-v0",
    entry_point="projectairsim.gym_envs.projectairsim_dronelanding_env:"
    "ProjectAirSimDroneLandingEnv",
)

register(
    id="ProjectAirSimDroneLandingSydney-v0",
    entry_point="projectairsim.gym_envs.projectairsim_dronelanding_sydney_env:"
    "ProjectAirSimDroneLandingSydneyEnv",
)

register(
    id="ProjectAirSimDroneLandingWithPerception-v0",
    entry_point="projectairsim.gym_envs.projectairsim_dronelanding_with_perception_env:"
    "ProjectAirSimDroneLandingWithPerceptionEnv",
)
