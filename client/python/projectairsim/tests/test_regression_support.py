"""Contracts that prevent host separation from hiding or reloading coverage."""
from copy import deepcopy
from types import SimpleNamespace
from unittest.mock import Mock

import pytest
import regression_support as support

pytestmark = pytest.mark.offline


def test_runtime_removes_rendered_sensors_without_mutating_source():
    original = support.read_scene("scene_test_drone_sensors.jsonc")
    saved = deepcopy(original)
    adapted = support.runtime_scene(original)
    assert original == saved
    sensors = adapted["actors"][0]["robot-config"]["sensors"]
    assert {s["type"] for s in sensors} == {
        "imu", "gps", "barometer", "magnetometer", "airspeed"}
    expected_clock = deepcopy(original["clock"])
    expected_clock["real-time-update-rate"] = 300_000
    assert adapted["clock"] == expected_clock
    assert adapted["clock"]["step-ns"] == original["clock"]["step-ns"] == 3_000_000
    assert adapted["actors"][0]["robot-config"]["controller"] == original["actors"][0]["robot-config"]["controller"]


def test_shared_unreal_scene_preserves_all_lidar_variants_and_camera_origins():
    scene = support.unreal_scene()
    sensors = scene["actors"][0]["robot-config"]["sensors"]
    ids = [s["id"] for s in sensors]
    assert len(ids) == len(set(ids))
    assert {s["id"] for s in sensors if s["type"] == "lidar"} == set(support.LIDARS)
    annotation = next(s for s in sensors if s["id"] == "SensorCamera")
    assert annotation["origin"]["xyz"] == "1.1 2.2 -3.3"
    assert annotation["annotation-settings"]["enabled"]
    assert len(scene["environment-actors"]) == 1


def test_attaching_client_never_reloads_scene_or_requests_topic_refresh(monkeypatch):
    world = SimpleNamespace(client=Mock(), parent_topic="/Sim/Shared", sim_config={})
    monkeypatch.setattr(support, "HOST", "unreal")
    monkeypatch.setattr(support, "SESSION", SimpleNamespace(world=world, topics={"camera": {"id": 1}}))
    client = Mock()
    attached = support.RegressionWorld(client, "scene_test_drone.jsonc")
    assert attached.parent_topic == world.parent_topic
    assert attached.client is client
    client.request.assert_not_called()
    client.get_topic_info.assert_not_called()
    client.topics["camera"]["id"] = 2
    assert support.SESSION.topics["camera"]["id"] == 1
    with pytest.raises(RuntimeError, match="must not reload"):
        attached.load_scene({})
    with pytest.raises(ValueError, match="explicitly"):
        support.RegressionWorld(client, "new_unreviewed_scene.jsonc")
