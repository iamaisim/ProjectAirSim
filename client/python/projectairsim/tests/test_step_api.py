"""
Tests for the pull API Step() method on World.
"""

from unittest.mock import MagicMock
from projectairsim.world import World


def _make_world():
    """Create a World with a mocked client, bypassing __init__ side effects."""
    world = World.__new__(World)
    world.client = MagicMock()
    world.parent_topic = "/Sim/TestScene"
    return world


def test_step_sends_correct_request():
    world = _make_world()
    world.client.request.return_value = {
        "sim_time_ns": 100_000_000,
        "robots": {},
    }

    result = world.step(dt_ns=100_000_000)

    world.client.request.assert_called_once()
    req = world.client.request.call_args[0][0]
    assert req["method"] == "/Sim/TestScene/Step"
    assert req["params"]["dt_ns"] == 100_000_000
    assert req["version"] == 1.0


def test_step_returns_sim_time_and_robots():
    world = _make_world()
    expected = {
        "sim_time_ns": 200_000_000,
        "robots": {
            "Drone1": {
                "state": {
                    "position": {"x": 1.0, "y": 2.0, "z": -3.0},
                    "orientation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
                    "linear_velocity": {"x": 0.5, "y": 0.0, "z": 0.0},
                    "angular_velocity": {"x": 0.0, "y": 0.0, "z": 0.1},
                },
                "events": [],
            }
        },
    }
    world.client.request.return_value = expected

    result = world.step(dt_ns=100_000_000)

    assert result["sim_time_ns"] == 200_000_000
    assert "Drone1" in result["robots"]
    assert result["robots"]["Drone1"]["state"]["position"]["x"] == 1.0
    assert result["robots"]["Drone1"]["events"] == []


def test_step_returns_collision_events():
    world = _make_world()
    expected = {
        "sim_time_ns": 300_000_000,
        "robots": {
            "Drone1": {
                "state": {
                    "position": {"x": 0.0, "y": 0.0, "z": 0.0},
                    "orientation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
                    "linear_velocity": {"x": 0.0, "y": 0.0, "z": 0.0},
                    "angular_velocity": {"x": 0.0, "y": 0.0, "z": 0.0},
                },
                "events": [
                    {
                        "type": "collision",
                        "sim_time_ns": 299_000_000,
                        "object_name": "gate_frame",
                        "impact_point": {"x": 10.0, "y": 0.0, "z": -2.0},
                        "normal": {"x": -1.0, "y": 0.0, "z": 0.0},
                    }
                ],
            }
        },
    }
    world.client.request.return_value = expected

    result = world.step(dt_ns=100_000_000)

    events = result["robots"]["Drone1"]["events"]
    assert len(events) == 1
    assert events[0]["type"] == "collision"
    assert events[0]["object_name"] == "gate_frame"


def test_step_returns_gate_pass_events():
    world = _make_world()
    expected = {
        "sim_time_ns": 400_000_000,
        "robots": {
            "Drone1": {
                "state": {
                    "position": {"x": 20.0, "y": 0.0, "z": -2.0},
                    "orientation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
                    "linear_velocity": {"x": 8.0, "y": 0.0, "z": 0.0},
                    "angular_velocity": {"x": 0.0, "y": 0.0, "z": 0.0},
                },
                "events": [
                    {
                        "type": "gate_pass",
                        "sim_time_ns": 398_000_000,
                        "gate_index": 3,
                        "lap_count": 1,
                        "is_correct_order": True,
                    }
                ],
            }
        },
    }
    world.client.request.return_value = expected

    result = world.step(dt_ns=100_000_000)

    events = result["robots"]["Drone1"]["events"]
    assert len(events) == 1
    assert events[0]["type"] == "gate_pass"
    assert events[0]["gate_index"] == 3
    assert events[0]["is_correct_order"] is True


def test_step_preserves_event_ordering():
    """Events within a step should preserve their sim_time_ns ordering."""
    world = _make_world()
    expected = {
        "sim_time_ns": 500_000_000,
        "robots": {
            "Drone1": {
                "state": {
                    "position": {"x": 0.0, "y": 0.0, "z": 0.0},
                    "orientation": {"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0},
                    "linear_velocity": {"x": 0.0, "y": 0.0, "z": 0.0},
                    "angular_velocity": {"x": 0.0, "y": 0.0, "z": 0.0},
                },
                "events": [
                    {
                        "type": "gate_pass",
                        "sim_time_ns": 498_000_000,
                        "gate_index": 2,
                        "lap_count": 1,
                        "is_correct_order": True,
                    },
                    {
                        "type": "collision",
                        "sim_time_ns": 499_000_000,
                        "object_name": "gate_frame",
                        "impact_point": {"x": 10.0, "y": 0.0, "z": -2.0},
                        "normal": {"x": -1.0, "y": 0.0, "z": 0.0},
                    },
                ],
            }
        },
    }
    world.client.request.return_value = expected

    result = world.step(dt_ns=100_000_000)

    events = result["robots"]["Drone1"]["events"]
    assert len(events) == 2
    assert events[0]["sim_time_ns"] < events[1]["sim_time_ns"]
    assert events[0]["type"] == "gate_pass"
    assert events[1]["type"] == "collision"
