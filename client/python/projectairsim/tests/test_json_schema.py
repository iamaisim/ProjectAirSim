"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Tests to validate errors from jsonschema validation
"""

import os
import time

import pytest
import jsonschema

from pynng import NNGException
from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import load_scene_config_as_dict

@pytest.fixture(scope="module", autouse=True)
def client(request):
    client = ProjectAirSimClient()
    return client


def test_scene_schema(client):
    with pytest.raises(jsonschema.exceptions.ValidationError) as error:
        world = World(client, 'scene_test_schema.jsonc')


def test_robot_required_schema(client):
    with pytest.raises(jsonschema.exceptions.ValidationError) as error:
        world = World(client, 'robot_test_required_schema.jsonc')

def test_robot_type_schema(client):
    with pytest.raises(jsonschema.exceptions.ValidationError) as error:
        world = World(client, 'robot_test_type_schema.jsonc')


def test_load_scene_config_respects_caller_sim_config_path():
    """Regression: sim_config_path provided by the caller must be used verbatim
    for path resolution of scene and robot config files."""
    config_name = "scene_test_drone.jsonc"
    sim_config_path = os.path.join(os.path.dirname(__file__), "sim_config")

    _, filepaths = load_scene_config_as_dict(config_name, sim_config_path)

    assert filepaths[0] == os.path.join(sim_config_path, config_name)
    assert filepaths[1] == [
        os.path.join(sim_config_path, "robot_test_quadrotor_fastphysics.jsonc")
    ]

pytestmark = pytest.mark.offline
