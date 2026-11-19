"""
Copyright (C) Microsoft Corporation. 
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

This script generates a DataGenerator object that can be access by all other tests in
the test_datacollection folder
"""
import pytest
from copy import deepcopy
from projectairsim.datacollection.collection import helper
from regression_support import RegressionWorld

from projectairsim.datacollection.data_generator import DataGenerator


@pytest.fixture(scope="session")
def data_generator_config():
    return DataGenerator(
        config_dir="./test_datacollection/configs",
    )


@pytest.fixture(scope="session")
def generated_data(unreal_session):
    generator = DataGenerator(config_dir="./test_datacollection/configs")
    # Path planning still queries real Unreal occupancy. Only replace the
    # scene-loading boundary so helper.setup_scene binds the shared scene.
    with pytest.MonkeyPatch.context() as patch:
        patch.setattr(helper, "World", RegressionWorld)
        generator.generate_trajectory()
    return generator


@pytest.fixture
def data_generator(generated_data):
    # API mutation tests must not change another test's trajectories or presets.
    with pytest.MonkeyPatch.context() as patch:
        patch.setattr(helper, "World", RegressionWorld)
        yield deepcopy(generated_data)
