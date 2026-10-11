"""Copyright (C) 2025 IAMAI CONSULTING CORP -- MIT License."""

import asyncio
import importlib.util
from pathlib import Path
import sys
from types import ModuleType, SimpleNamespace
from unittest.mock import AsyncMock, Mock, call

import pytest

pytestmark = pytest.mark.offline


@pytest.fixture
def example(monkeypatch):
    events = []
    state = SimpleNamespace(failed_operation=None)
    engine = Mock()

    def set_param(model, parameter, operation, **kwargs):
        events.append(operation)

        def result(**kwargs):
            events.append(operation + "_result")
            if operation == state.failed_operation:
                raise state.engine_error

        return Mock(result=Mock(side_effect=result))

    engine.set_param.side_effect = set_param
    matlab = ModuleType("matlab")
    matlab.engine = ModuleType("matlab.engine")
    matlab.engine.connect_matlab = Mock(return_value=engine)
    matlab.engine.find_matlab = Mock(return_value=("MATLABEngine",))
    monkeypatch.setitem(sys.modules, "matlab", matlab)
    monkeypatch.setitem(sys.modules, "matlab.engine", matlab.engine)
    path = (Path(__file__).resolve().parents[2] / "example_user_scripts" /
            "simulink_external_quadrotor.py")
    spec = importlib.util.spec_from_file_location("external_quadrotor_example", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    client = Mock()
    client.connect.side_effect = lambda: events.append("connect")
    client.disconnect.side_effect = lambda: events.append("disconnect")
    world = Mock()
    world.pause.side_effect = lambda: events.append("pause")
    drone = Mock()
    drone.enable_api_control.return_value = True
    drone.can_arm.return_value = True
    drone.arm.return_value = True

    async def completed_command(*args, **kwargs):
        task = asyncio.get_running_loop().create_future()
        task.set_result(True)
        return task

    for name in ("hover_async", "move_by_velocity_async",
                 "move_by_velocity_body_frame_async", "move_to_position_async",
                 "land_async"):
        setattr(drone, name, AsyncMock(side_effect=completed_command))

    def make_world(*args, **kwargs):
        events.append("world")
        return world

    def make_drone(*args, **kwargs):
        events.append("drone")
        return drone

    monkeypatch.setattr(module, "ProjectAirSimClient", Mock(return_value=client))
    monkeypatch.setattr(module, "World", Mock(side_effect=make_world))
    monkeypatch.setattr(module, "Drone", Mock(side_effect=make_drone))
    monkeypatch.setattr(module, "projectairsim_log", Mock(return_value=Mock()))
    state.matlab = matlab
    state.module = module
    state.client = client
    state.world = world
    state.drone = drone
    state.engine = engine
    state.events = events
    return state


def test_compilation_precedes_connection_and_flight_sequence_is_preserved(example):
    asyncio.run(example.module.main())
    assert example.events == [
        "update", "update_result", "connect", "start", "world", "drone",
        "start_result", "stop", "stop_result", "pause", "disconnect",
    ]
    assert example.drone.mock_calls == [
        call.enable_api_control(), call.can_arm(), call.arm(),
        call.hover_async(), call.move_by_velocity_async(1, 0, 0, duration=3),
        call.move_by_velocity_body_frame_async(0, 1, 0, duration=3),
        call.move_to_position_async(0, 0, -4, velocity=1, timeout_sec=30),
        call.land_async(timeout_sec=30), call.cancel_last_task(),
        call.disarm(), call.disable_api_control(),
    ]


@pytest.mark.parametrize("cleanup", ["cancel_last_task", "disarm",
                                      "disable_api_control"])
def test_drone_cleanup_failure_preserves_flight_error_and_continues(example, cleanup):
    flight_error = RuntimeError("Flight command failed")
    example.drone.move_to_position_async.side_effect = flight_error
    getattr(example.drone, cleanup).side_effect = RuntimeError("Cleanup failed")
    with pytest.raises(RuntimeError) as error:
        asyncio.run(example.module.main())
    assert error.value is flight_error
    example.drone.cancel_last_task.assert_called_once_with()
    example.drone.disarm.assert_called_once_with()
    example.drone.disable_api_control.assert_called_once_with()
    assert example.events[-4:] == ["stop", "stop_result", "pause", "disconnect"]
    example.world.pause.assert_called_once_with()
    example.client.disconnect.assert_called_once_with()


def test_model_stop_failure_still_pauses_world_and_disconnects(example):
    example.failed_operation = "stop"
    example.engine_error = RuntimeError("Model stop failed")
    with pytest.raises(RuntimeError) as error:
        asyncio.run(example.module.main())
    assert error.value is example.engine_error
    assert example.events[-4:] == ["stop", "stop_result", "pause", "disconnect"]
    example.world.pause.assert_called_once_with()
    example.client.disconnect.assert_called_once_with()


def test_compilation_failure_prevents_scene_start_and_preserves_error(example):
    example.failed_operation = "update"
    example.engine_error = RuntimeError("Model compilation failed")
    with pytest.raises(RuntimeError) as error:
        asyncio.run(example.module.main())
    assert error.value is example.engine_error
    assert example.events == ["update", "update_result", "stop", "stop_result",
                              "disconnect"]
    example.client.connect.assert_not_called()
    example.module.World.assert_not_called()
    example.client.disconnect.assert_called_once_with()


@pytest.mark.parametrize("sessions, selected", [
    (("MATLAB_123",), "MATLAB_123"),
    (("MATLABEngine",), "MATLABEngine"),
])
def test_example_selects_unique_shared_session(example, sessions, selected):
    example.matlab.engine.find_matlab.return_value = sessions
    asyncio.run(example.module.main())
    example.matlab.engine.connect_matlab.assert_called_once_with(selected)


@pytest.mark.parametrize("sessions", [(), ("MATLAB_123", "MATLAB_456"),
                                      ("MATLAB_123", "MATLABEngine")])
def test_missing_or_ambiguous_session_prevents_connection(example, sessions):
    example.matlab.engine.find_matlab.return_value = sessions
    with pytest.raises(RuntimeError, match="shared MATLAB session"):
        asyncio.run(example.module.main())
    example.matlab.engine.connect_matlab.assert_not_called()
    example.client.connect.assert_not_called()
