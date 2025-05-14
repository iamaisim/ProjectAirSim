"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

This script generates a DataGenerator object that can be access by all other tests in
the test_datacollection folder
"""
import pytest

from projectairsim.datacollection.data_generator import DataGenerator


@pytest.fixture(scope="session")
def data_generator():
    data_generator = DataGenerator(
        config_dir="./test_datacollection/configs",
    )
    _ = data_generator.generate_trajectory()
    return data_generator
