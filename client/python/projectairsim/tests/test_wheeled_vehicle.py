"""Unit tests for the WheeledVehicle client request contract."""

from pathlib import Path
from unittest.mock import MagicMock

import commentjson
import jsonschema
import pytest

from projectairsim import WheeledVehicle


def make_vehicle():
    client = MagicMock()
    client.request.return_value = True
    world = MagicMock()
    world.parent_topic = "/Sim/TestScene"
    world.get_configuration.return_value = {
        "actors": [{"name": "Car1", "robot-config": {}}]
    }
    return WheeledVehicle(client, world, "Car1"), client


@pytest.mark.parametrize(
    "method,rpc,value",
    [
        ("set_throttle", "SetThrottle", 0.7),
        ("set_steering", "SetSteering", -0.25),
        ("set_brakes", "SetBrakes", 1.0),
    ],
)
@pytest.mark.parametrize("result", [True, False])
def test_named_controls_send_specific_rpc(method, rpc, value, result):
    vehicle, client = make_vehicle()
    client.request.return_value = result

    assert getattr(vehicle, method)(value) is result

    client.request.assert_called_once_with(
        {
            "method": f"/Sim/TestScene/robots/Car1/{rpc}",
            "params": {"value": value},
            "version": 1.0,
        }
    )


def test_wheeled_vehicle_does_not_expose_set_parameter():
    vehicle, client = make_vehicle()
    assert not hasattr(vehicle, "set_parameter")
    client.request.assert_not_called()


@pytest.mark.parametrize("method", ["set_throttle", "set_steering", "set_brakes"])
def test_named_controls_propagate_rpc_errors(method):
    vehicle, client = make_vehicle()
    client.request.side_effect = RuntimeError("RPC rejected")
    with pytest.raises(RuntimeError, match="RPC rejected"):
        getattr(vehicle, method)(0.5)


def test_wheeled_vehicle_is_exported_from_package():
    assert WheeledVehicle.__name__ == "WheeledVehicle"


def test_wheeled_vehicle_config_schema_and_exclusivity():
    projectairsim_root = Path(__file__).parents[1]
    schema_path = (
        projectairsim_root / "src/projectairsim/schema/robot_config_schema.jsonc"
    )
    with schema_path.open() as stream:
        schema = commentjson.load(stream)
    with (
        projectairsim_root.parent
        / "example_user_scripts/sim_config/robot_wheeled_vehicle.jsonc"
    ).open() as stream:
        config = commentjson.load(stream)

    jsonschema.validate(config, schema)

    config["unreal-vehicle-class"] = "/Game/Other.Other_C"
    with pytest.raises(jsonschema.ValidationError):
        jsonschema.validate(config, schema)


def test_wheeled_vehicle_simpledrive_config_needs_no_actuators():
    projectairsim_root = Path(__file__).parents[1]
    schema_path = (
        projectairsim_root / "src/projectairsim/schema/robot_config_schema.jsonc"
    )
    with schema_path.open() as stream:
        schema = commentjson.load(stream)
    with (
        projectairsim_root.parent
        / "example_user_scripts/sim_config/robot_wheeled_vehicle_simpledrive.jsonc"
    ).open() as stream:
        config = commentjson.load(stream)

    jsonschema.validate(config, schema)
    assert config["controller"]["type"] == "simple-drive-api"
    assert "actuators" not in config


def test_wheeled_vehicle_simpledrive_scene_uses_physics_substeps():
    projectairsim_root = Path(__file__).parents[1]
    with (
        projectairsim_root.parent
        / "example_user_scripts/sim_config/scene_wheeled_vehicle_simpledrive.jsonc"
    ).open() as stream:
        config = commentjson.load(stream)

    # Treat 3 ms as a Chaos substep ceiling. Without this switch Unreal maps it
    # to the render-frame delta and demands 333 FPS, causing slow motion.
    assert config["clock"]["type"] == "engine-driven"
    assert config["clock"]["step-ns"] == 3_000_000
    assert config["clock"]["engine-substepping"] is True
    assert "real-time-update-rate" not in config["clock"]
