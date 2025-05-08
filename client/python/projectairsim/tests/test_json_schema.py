"""
Copyright (C) Microsoft Corporation. All rights reserved.
Tests to validate errors from jsonschema validation
"""

import time

import pytest
import jsonschema

from pynng import NNGException
from projectairsim import Drone, ProjectAirSimClient, World

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


