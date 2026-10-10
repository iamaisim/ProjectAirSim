"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Offline lifecycle regressions for the stock Simulink examples.
"""

import asyncio
import importlib.util
from pathlib import Path
import sys
from types import ModuleType, SimpleNamespace

import pytest

pytestmark = pytest.mark.offline

_EXAMPLES = Path(__file__).resolve().parents[2] / "example_user_scripts"
_SCRIPTS = (
    "simulink_physics_quadrotor",
    "simulink_physics_quadtiltrotor",
    "simulink_controller_quadrotor",
    "simulink_combined_quadrotor",
)


@pytest.fixture(params=_SCRIPTS)
def example(request, monkeypatch):
    events = []
    failures = {}

    def operation(name):
        events.append(name)
        if name in failures:
            raise failures[name]

    class Future:
        def __init__(self, command):
            self.command = command
            self.finished = False
            self.was_cancelled = False

        def result(self, timeout=None):
            if timeout is None or timeout <= 0:
                raise AssertionError("Model lifecycle waits must be bounded.")
            if self.command + "_interrupted" in failures:
                # MATLAB Engine consumes Ctrl+C and returns None after cancellation.
                self.was_cancelled = True
                self.finished = True
                return None
            if not isinstance(failures.get(self.command + "_completed"), TimeoutError):
                self.finished = True
            operation(self.command + "_completed")

        def done(self):
            return self.finished

        def cancelled(self):
            return self.was_cancelled

        def cancel(self):
            operation(self.command + "_cancelled")
            self.was_cancelled = True
            self.finished = True
            return True

    class Engine:
        def set_param(self, model, parameter, command, **kwargs):
            assert model == "model" and parameter == "SimulationCommand"
            operation(command)
            assert kwargs == {"nargout": 0, "background": True}
            return Future(command)

        def quit(self):
            pytest.fail("The example must preserve the shared MATLAB desktop.")

    engine = Engine()
    matlab = ModuleType("matlab")
    matlab.engine = ModuleType("matlab.engine")
    matlab.engine.connect_matlab = lambda name: operation("engine_connect") or engine
    monkeypatch.setitem(sys.modules, "matlab", matlab)
    monkeypatch.setitem(sys.modules, "matlab.engine", matlab.engine)
    monkeypatch.syspath_prepend(str(_EXAMPLES))
    spec = importlib.util.spec_from_file_location(request.param, _EXAMPLES / (request.param + ".py"))
    script = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(script)

    class Client:
        def __init__(self, server):
            operation("client_created")

        def connect(self):
            operation("client_connect")

        def disconnect(self):
            operation("client_disconnect")

    class Drone:
        VTOLMode = SimpleNamespace(FixedWing=1, Multirotor=0)

        def __init__(self, *args):
            operation("drone_created")

        def enable_api_control(self):
            operation("api_control")

        def arm(self):
            operation("arm")

        def cancel_last_task(self):
            operation("cancel")
            return True

        def disarm(self):
            operation("disarm")

        def disable_api_control(self):
            operation("release")

        def get_ground_truth_kinematics(self):
            return {"pose": {"position": {"x": 0, "y": 0, "z": -4}}}

        def __getattr__(self, name):
            async def command(*args, **kwargs):
                operation("flight")

                async def complete():
                    return True

                return complete()

            return command

    async def sleep(duration):
        pass

    monkeypatch.setattr(script, "ProjectAirSimClient", Client)
    monkeypatch.setattr(script, "World", lambda *args, **kwargs: operation("scene_loaded"))
    monkeypatch.setattr(script, "Drone", Drone)
    monkeypatch.setattr(script.asyncio, "sleep", sleep)
    monkeypatch.setattr("builtins.input", lambda *args: operation("flight"))
    return SimpleNamespace(
        run=lambda: asyncio.run(script.main("scene.jsonc", "model")),
        events=events, failures=failures,
        flies="physics" in request.param,
    )


def test_model_compiles_before_starting_the_scene(example):
    example.run()
    assert example.events.index("update_completed") < example.events.index("start")
    assert example.events.index("start") < example.events.index("scene_loaded")
    assert example.events.index("start_completed") < example.events.index("flight")
    assert example.events[-3:] == ["stop", "stop_completed", "client_disconnect"]
    if not example.flies:
        assert not any(action in example.events for action in ("cancel", "disarm", "release"))


@pytest.mark.parametrize("stage", [
    "engine_connect", "update_completed", "start", "client_created", "client_connect", "start_completed",
])
def test_startup_failure_releases_acquired_resources(example, stage):
    failure = RuntimeError("startup failed")
    example.failures[stage] = failure
    with pytest.raises(RuntimeError) as caught:
        example.run()
    assert caught.value is failure
    if "start" in example.events:
        assert "stop_completed" in example.events
    else:
        assert "stop" not in example.events
    if "client_connect" in example.events:
        assert example.events[-1] == "client_disconnect"
    if stage == "update_completed":
        assert "client_created" not in example.events


@pytest.mark.parametrize("stage", ["cancel", "disarm", "release", "stop_completed"])
def test_cleanup_failure_does_not_skip_later_cleanup(example, stage):
    if stage in ("cancel", "disarm", "release") and not example.flies:
        pytest.skip("The controller-only examples do not acquire flight authority.")
    failure = RuntimeError("cleanup failed")
    example.failures[stage] = failure
    with pytest.raises(RuntimeError) as caught:
        example.run()
    assert caught.value is failure
    assert example.events[-1] == "client_disconnect"
    assert "stop_completed" in example.events
    if example.flies:
        assert all(action in example.events for action in ("cancel", "disarm", "release"))


@pytest.mark.parametrize("failure", [RuntimeError("flight failed"), KeyboardInterrupt(), asyncio.CancelledError()])
def test_interruption_preserves_original_failure_during_cleanup(example, failure):
    example.failures.update(flight=failure, stop_completed=RuntimeError("stop failed"))
    with pytest.raises(type(failure)) as caught:
        example.run()
    assert caught.value is failure
    assert example.events[-1] == "client_disconnect"
    assert "stop_completed" in example.events


@pytest.mark.parametrize("command", ["update", "start"])
def test_engine_cancellation_does_not_continue_to_flight(example, command):
    example.failures[command + "_interrupted"] = KeyboardInterrupt()
    with pytest.raises(KeyboardInterrupt):
        example.run()
    assert "flight" not in example.events
    if command == "update":
        assert "start" not in example.events
    else:
        assert example.events[-1] == "client_disconnect"


@pytest.mark.parametrize("command", ["update", "start", "stop"])
def test_timeout_cancels_pending_matlab_call(example, command):
    failure = TimeoutError("MATLAB call timed out")
    example.failures[command + "_completed"] = failure
    with pytest.raises(TimeoutError) as caught:
        example.run()
    assert caught.value is failure
    assert command + "_cancelled" in example.events
    if command != "update":
        assert example.events[-1] == "client_disconnect"


def test_scene_failure_cancels_pending_model_start(example):
    failure = RuntimeError("scene load failed")
    example.failures["scene_loaded"] = failure
    with pytest.raises(RuntimeError) as caught:
        example.run()
    assert caught.value is failure
    assert example.events.index("start_cancelled") < example.events.index("stop")
    assert example.events[-1] == "client_disconnect"


def test_interrupt_during_cleanup_still_stops_and_disconnects(example):
    if not example.flies:
        pytest.skip("The controller-only examples do not acquire flight authority.")
    example.failures["cancel"] = KeyboardInterrupt()
    with pytest.raises(KeyboardInterrupt):
        example.run()
    assert "stop_completed" in example.events
    assert example.events[-1] == "client_disconnect"
