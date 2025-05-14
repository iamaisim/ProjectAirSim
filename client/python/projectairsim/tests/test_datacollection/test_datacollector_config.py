"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Tests for the datacollection module config and its schema 
"""

import commentjson
import jsonschema
import pytest


def read_cjson(json_file: str):
    with open(json_file) as f:
        data = commentjson.load(f)
    return data


@pytest.fixture(scope="module")
def config_main():
    config_path = (
        r"../../example_user_scripts/datacollection/configs/datacollector_config.jsonc"
    )
    config = read_cjson(config_path)
    return config


@pytest.fixture(scope="module")
def config_test():
    config_path = r"./test_datacollection/configs/datacollector_config.jsonc"
    config = read_cjson(config_path)
    return config


@pytest.fixture(scope="module")
def schema():
    config_path = r"../../example_user_scripts/datacollection/configs/schemas/datacollector_config_schema.jsonc"
    config = read_cjson(config_path)
    return config


def test_valid_config_test(config_test, schema):
    try:
        jsonschema.validate(instance=config_test, schema=schema)
    except jsonschema.exceptions.ValidationError as err:
        raise err


def test_valid_config_main(config_main, schema):
    try:
        jsonschema.validate(instance=config_main, schema=schema)
    except jsonschema.exceptions.ValidationError as err:
        raise err
