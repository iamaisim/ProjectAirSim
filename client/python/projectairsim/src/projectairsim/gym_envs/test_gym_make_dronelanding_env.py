import gym
import os
import sys
import asyncio
import pytest

# path to projectairsim module
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))

import projectairsim.gym_envs  # noqa: F401 E402
from gym.spaces import Box  # noqa: E402

@pytest.mark.skip(reason="Skipped until timeout issues due to float precision is fixed")
def test_gym_make_projectairsim_dronelanding_with_images_env():
    env = gym.make("ProjectAirSimDroneLandingWithImages-v0")
    assert env.action_space
    assert isinstance(env.action_space, Box)
    assert env.observation_space
    assert isinstance(env.observation_space, Box)
    env.close()


@pytest.mark.skip(reason="Skipped until timeout issues due to float precision is fixed")
def test_gym_make_projectairsim_dronelanding_env():
    env = gym.make("ProjectAirSimDroneLanding-v0")
    assert env.action_space
    assert isinstance(env.action_space, Box)
    assert env.observation_space
    assert isinstance(env.observation_space, Box)
    env.close()


@pytest.mark.skip(reason="Skipped until timeout issues due to float precision is fixed")
def test_gym_run_projectairsim_dronelanding_env():
    # Run projectairsim_dronelanding_env.py as a pytest
    import projectairsim.gym_envs.projectairsim_dronelanding_env

    asyncio.run(projectairsim.gym_envs.projectairsim_dronelanding_env.main())


@pytest.mark.skip(reason="Skipped until timeout issues due to float precision is fixed")
def test_gym_run_projectairsim_dronelanding_with_images_env():
    # Run projectairsim_dronelanding_with_images_env.py as a pytest
    import projectairsim.gym_envs.projectairsim_dronelanding_with_images_env

    asyncio.run(projectairsim.gym_envs.projectairsim_dronelanding_with_images_env.main())
