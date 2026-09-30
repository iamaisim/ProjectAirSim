"""
Copyright (C) 2025 IAMAI CONSULTING CORP -- MIT License.
Pytest configuration for projectairsim integration tests.
"""
import pytest
import regression_support
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from threading import Thread
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim import ProjectAirSimClient


def pytest_addoption(parser):
    parser.addoption("--sim-host", choices=("offline", "runtime", "unreal", "px4"),
                     default="unreal", help="Select one disjoint regression suite")
    parser.addoption(
        "--plot",
        action="store_true",
        default=False,
        help="Show real-time 3D matplotlib viewer during flight",
    )


def pytest_configure(config):
    regression_support.HOST = config.getoption("--sim-host")


def pytest_collection_modifyitems(config, items):
    host = config.getoption("--sim-host")
    selected, deselected = [], []
    for item in items:
        hosts = [name for name in ("offline", "runtime", "unreal", "px4")
                 if item.get_closest_marker(name)]
        if len(hosts) != 1:
            raise pytest.UsageError(f"{item.nodeid} must declare exactly one regression host")
        (selected if hosts[0] == host else deselected).append(item)
    items[:] = selected
    config.hook.pytest_deselected(items=deselected)


@pytest.fixture(scope="session", autouse=True)
def unreal_session(request):
    if request.config.getoption("--sim-host") == "offline":
        def reject_connection(*args, **kwargs):
            raise AssertionError("An offline regression attempted to connect to a simulator")
        with pytest.MonkeyPatch.context() as patch:
            patch.setattr(ProjectAirSimClient, "connect", reject_connection)
            patch.setattr(ProjectAirSimClient, "connect_services", reject_connection)
            yield None
        return
    if request.config.getoption("--sim-host") != "unreal":
        yield None
        return
    original_request = ProjectAirSimClient.request
    request.config._unreal_scene_loads = 0

    def guard_scene_load(client, message, *args, **kwargs):
        if message.get("method") == "/Sim/LoadScene":
            if request.config._unreal_scene_loads:
                raise AssertionError("A regression attempted to reload the shared Unreal scene")
            request.config._unreal_scene_loads += 1
        return original_request(client, message, *args, **kwargs)

    with pytest.MonkeyPatch.context() as patch:
        patch.setattr(ProjectAirSimClient, "request", guard_scene_load)
        session = regression_support.UnrealSession()
        regression_support.SESSION = session
        yield session
        try:
            session.close()
        finally:
            regression_support.SESSION = None


@pytest.fixture(autouse=True)
def isolate_unreal_test(request):
    if request.node.get_closest_marker("unreal"):
        session = request.getfixturevalue("unreal_session")
        session.reset()
        try:
            yield
        finally:
            session.reset(restore_segmentation=request.node.name == "test_get_set_segmentation_id")
    else:
        yield


@pytest.fixture
def material_object(unreal_session, isolate_unreal_test):
    world = unreal_session.world
    pose = Pose({"translation": Vector3({"x": -30, "y": 0, "z": -4}),
                 "rotation": Quaternion({"w": 1, "x": 0, "y": 0, "z": 0}),
                 "frame_id": "DEFAULT_ID"})
    name = world.spawn_object("RegressionMaterial", "OrangeBall_Blueprint", pose,
                              [1, 1, 1], False)
    assert name
    try:
        yield name
    finally:
        world.destroy_object(name)


@pytest.fixture
def texture_url():
    handler = partial(SimpleHTTPRequestHandler,
                      directory=str(regression_support.CONFIG.parent / "assets"))
    server = ThreadingHTTPServer(("127.0.0.1", 0), handler)
    thread = Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        yield f"http://127.0.0.1:{server.server_port}/sample_texture.png"
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=5)


def pytest_terminal_summary(terminalreporter):
    if terminalreporter.config.getoption("--sim-host") == "unreal":
        loads = getattr(terminalreporter.config, "_unreal_scene_loads", 0)
        terminalreporter.write_line(f"Unreal regression scene loads: {loads}")
