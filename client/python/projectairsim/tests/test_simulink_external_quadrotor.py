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
    state = SimpleNamespace(failed_operation=None, interrupted=set(),
                            late_interrupt=set())
    engine = Mock()

    class Future:
        def __init__(self, operation, value=None):
            self.operation = operation
            self.value = value
            self.error = None
            self.finished = False
            self.was_cancelled = False
            self.retrieved = False

        def result(self, timeout=None):
            assert timeout is not None and timeout > 0
            if self.retrieved:
                if self.error is not None:
                    raise self.error
                return self.value
            events.append(self.operation + "_result")
            self.finished = True
            if self.operation in state.interrupted:
                # MATLAB Engine consumes Ctrl+C and returns None after cancelling.
                self.was_cancelled = True
                return None
            self.retrieved = True
            if self.operation == state.failed_operation:
                self.error = state.engine_error
                if self.operation in state.late_interrupt:
                    # Ctrl+C after completion: cancellation fails, None is returned.
                    return None
                raise self.error
            return self.value

        def done(self):
            return self.finished

        def cancelled(self):
            return self.was_cancelled

        def cancel(self):
            events.append(self.operation + "_cancelled")
            self.was_cancelled = True
            self.finished = True
            return True

    def set_param(model, parameter, operation, **kwargs):
        assert kwargs.get("background")
        events.append(operation)
        return Future(operation)

    engine.set_param.side_effect = set_param
    matlab = ModuleType("matlab")
    matlab.engine = ModuleType("matlab.engine")
    matlab.engine.connect_matlab = Mock(return_value=Future("matlab", engine))
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
        "matlab_result", "update", "update_result", "connect", "start", "world",
        "drone", "start_result", "stop", "stop_result", "pause", "disconnect",
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
    assert example.events == ["matlab_result", "update", "update_result", "stop",
                              "stop_result", "disconnect"]
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
    example.matlab.engine.connect_matlab.assert_called_once_with(
        selected, background=True)


@pytest.mark.parametrize("sessions", [(), ("MATLAB_123", "MATLAB_456"),
                                      ("MATLAB_123", "MATLABEngine")])
def test_missing_or_ambiguous_session_prevents_connection(example, sessions):
    example.matlab.engine.find_matlab.return_value = sessions
    with pytest.raises(RuntimeError, match="shared MATLAB session"):
        asyncio.run(example.module.main())
    example.matlab.engine.connect_matlab.assert_not_called()
    example.client.connect.assert_not_called()


def test_connection_failure_explains_matlab_setup(example):
    example.failed_operation = "matlab"
    example.engine_error = RuntimeError("MATLAB session not found")
    with pytest.raises(RuntimeError, match="load_external_quadrotor") as error:
        asyncio.run(example.module.main())
    assert error.value.__cause__ is example.engine_error
    example.module.ProjectAirSimClient.assert_not_called()


@pytest.mark.parametrize("operation", ["matlab", "update", "start"])
def test_engine_cancellation_stops_before_flight(example, operation):
    example.interrupted.add(operation)
    with pytest.raises(KeyboardInterrupt):
        asyncio.run(example.module.main())
    example.drone.enable_api_control.assert_not_called()
    example.drone.hover_async.assert_not_called()
    if operation == "matlab":
        example.engine.set_param.assert_not_called()
        example.module.ProjectAirSimClient.assert_not_called()
    else:
        assert example.events[-1] == "disconnect"
        assert "stop_result" in example.events
    if operation == "update":
        example.client.connect.assert_not_called()


@pytest.mark.parametrize("operation", ["update", "start", "stop"])
def test_interrupt_after_completion_reports_matlab_error(example, operation):
    example.failed_operation = operation
    example.late_interrupt.add(operation)
    example.engine_error = RuntimeError("MATLAB command failed")
    with pytest.raises(RuntimeError) as error:
        asyncio.run(example.module.main())
    assert error.value is example.engine_error
    if operation != "stop":
        example.drone.hover_async.assert_not_called()
    assert example.events[-1] == "disconnect"


def test_interrupted_cleanup_step_continues_and_preserves_error(example):
    flight_error = RuntimeError("Flight command failed")
    example.drone.move_to_position_async.side_effect = flight_error
    example.drone.disarm.side_effect = KeyboardInterrupt()
    with pytest.raises(RuntimeError) as error:
        asyncio.run(example.module.main())
    assert error.value is flight_error
    example.drone.disable_api_control.assert_called_once_with()
    assert example.events[-4:] == ["stop", "stop_result", "pause", "disconnect"]


def test_scene_failure_cancels_pending_model_start(example):
    scene_error = RuntimeError("Scene load failed")
    example.module.World.side_effect = scene_error
    with pytest.raises(RuntimeError) as error:
        asyncio.run(example.module.main())
    assert error.value is scene_error
    assert example.events[-4:] == ["start_cancelled", "stop", "stop_result",
                                   "disconnect"]
