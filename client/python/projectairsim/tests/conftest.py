"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Global pytest configuration for ProjectAirSim tests.
Provides test isolation and stability improvements.
"""

import pytest
import time
import gc


def _socket_is_closed(socket_obj) -> bool:
    if socket_obj is None:
        return True
    return bool(getattr(socket_obj, "closed", False))


def _client_needs_reconnect(client) -> bool:
    if client is None:
        return False
    if not getattr(client, "state", False):
        return True
    if _socket_is_closed(getattr(client, "socket_topics", None)):
        return True
    if _socket_is_closed(getattr(client, "socket_services", None)):
        return True
    return False


def _safe_disconnect(client) -> None:
    if client is None:
        return
    try:
        client.disconnect()
    except Exception:
        pass


def _reload_world_if_possible(world) -> None:
    if world is None:
        return
    sim_config = getattr(world, "sim_config", None)
    if sim_config is None:
        return
    world.load_scene(sim_config, delay_after_load_sec=1.0)


def _extract_client_and_world(request):
    client = None
    world = None

    for fixture_name in ("client", "world", "drone", "multirotor", "robo_fixture", "robo"):
        if fixture_name not in request.fixturenames:
            continue

        try:
            fixture_obj = request.getfixturevalue(fixture_name)
        except Exception:
            continue

        if fixture_name == "client":
            client = fixture_obj
        elif fixture_name == "world":
            world = fixture_obj
            client = getattr(fixture_obj, "client", client)
        else:
            client = getattr(fixture_obj, "client", client)
            world = getattr(fixture_obj, "world", world)

    return client, world


@pytest.fixture(scope="function", autouse=True)
def ensure_client_connected(request):
    client, world = _extract_client_and_world(request)

    if _client_needs_reconnect(client):
        _safe_disconnect(client)
        client.connect()
        try:
            client.get_topic_info()
        except Exception:
            pass
        _reload_world_if_possible(world)

    yield


@pytest.fixture(scope="function", autouse=True)
def test_isolation(request):
    """
    Automatic fixture to improve test isolation.
    
    Adds a delay after each test to allow the simulator to settle,
    preventing race conditions and state pollution between sequential tests.
    This is especially important for tests that:
    - Load/unload scenes
    - Modify textures/materials
    - Control drone movements
    - Capture images
    
    The delay can be skipped by marking a test with @pytest.mark.no_isolation
    """
    # Before test: force garbage collection to clean up any lingering objects
    gc.collect()
    
    yield
    
    # Check if test is marked to skip isolation delay
    if "no_isolation" in request.keywords:
        return
    
    # After test: add delay to allow simulator to settle
    # This prevents issues with:
    # - Scene loading timeouts
    # - Texture/material update propagation
    # - Image capture timing
    time.sleep(1.5)
    
    # Force garbage collection after test
    gc.collect()


@pytest.fixture(scope="session", autouse=True)
def session_setup_teardown():
    """
    Session-level setup and teardown.
    Runs once before all tests and once after all tests complete.
    """
    print("\n" + "="*80)
    print("Starting ProjectAirSim test session")
    print("Tests will run sequentially with isolation delays")
    print("="*80 + "\n")
    
    yield
    
    print("\n" + "="*80)
    print("ProjectAirSim test session complete")
    print("="*80 + "\n")


@pytest.fixture(scope="module")
def module_isolation():
    """
    Module-level isolation fixture.
    Add longer delay between test modules to allow major state resets.
    """
    yield
    # Longer delay between modules
    print("\n--- Module complete, allowing simulator to reset ---")
    time.sleep(3.0)


# Custom pytest markers for test categorization
def pytest_configure(config):
    """Register custom markers for test categorization."""
    config.addinivalue_line(
        "markers", "no_isolation: skip automatic test isolation delay"
    )
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )
    config.addinivalue_line(
        "markers", "requires_scene(name): marks tests that require specific scene loaded"
    )


def pytest_runtest_makereport(item, call):
    """
    Hook to track test failures and potentially adjust isolation behavior.
    """
    if call.when == "call":
        # Could add custom logic here to increase delays after failures
        pass

