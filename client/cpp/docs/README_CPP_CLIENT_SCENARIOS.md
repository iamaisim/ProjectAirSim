# ProjectAirSim C++ User Scenario Apps

This document describes the recommended workflow for end users: create or customize a single C++ app for your scenario and run only that app.

## Scope and Layout

Following the same organization principle used by the Python client (client-specific assets under `client/python/...`), all C++ client scenario assets are under `client/cpp/...`:

- Scripts: `client/cpp/scripts/`
- Docs: `client/cpp/docs/`
- Scenario app source: `client/cpp/example_user_apps/CppClientScenarios/`
- User app template source: `client/cpp/example_user_apps/UserScenarioTemplate/`
- Built binary: `client/cpp/build_local/cpp_client_scenarios`
- Built user-template binary: `client/cpp/build_local/user_scenario_template`

## Prerequisites

1. Simulation server running (default host `127.0.0.1`).
2. C++ client built in `client/cpp/build_local`.
3. Scene configs available in `client/python/example_user_scripts/sim_config/`.

## Build

```bash
cmake -S client/cpp -B client/cpp/build_local -DCMAKE_BUILD_TYPE=Debug
cmake --build client/cpp/build_local --target cpp_client_scenarios -j$(nproc)
cmake --build client/cpp/build_local --target user_scenario_template -j$(nproc)
```

## Recommended: Run One User Scenario App

Use the dedicated runner for single-app execution:

```bash
./client/cpp/scripts/run_cpp_user_scenario.sh
```

Common options:

```bash
./client/cpp/scripts/run_cpp_user_scenario.sh \
  --simhost 127.0.0.1 \
  --simconfig client/python/example_user_scripts/sim_config \
  --scene scene_basic_drone.jsonc \
  --vehicle Drone1
```

If you create a custom executable target, pass it with `--target`:

```bash
./client/cpp/scripts/run_cpp_user_scenario.sh --target my_custom_scenario_app
```

Rover-specific example:

```bash
./client/cpp/scripts/run_cpp_user_scenario.sh \
  --target user_rover_scenario \
  --scene scene_basic_rover.jsonc \
  --vehicle Rover1
```

## Creating Your Own Scenario App

1. Copy `client/cpp/example_user_apps/UserScenarioTemplate/UserScenarioTemplate.cpp`.
2. Rename the copied file and add your own logic inside `RunUserScenario(...)`.
3. Add a new `add_executable(...)` entry to `client/cpp/CMakeLists.txt`.
4. Link your target to `ProjectAirsimClient` (and `Threads::Threads` on Linux).
5. Build and run with `run_cpp_user_scenario.sh --target <your_target>`.

## Legacy Multi-Scenario Script

The old orchestrator script is still available:

```bash
./client/cpp/scripts/run_cpp_client_scenarios.sh --only basic
```

It now requires `--only` and no longer runs all scenarios by default.

## Legacy Scenario Wrappers

```bash
./client/cpp/scripts/run_cpp_client_scenario_basic.sh
./client/cpp/scripts/run_cpp_client_scenario_sensors.sh
./client/cpp/scripts/run_cpp_client_scenario_two_drones.sh
./client/cpp/scripts/run_cpp_client_scenario_wind.sh
./client/cpp/scripts/run_cpp_client_scenario_battery.sh
```

## Scenario Coverage

- `basic`: API control, arm/disarm, takeoff, move, land.
- `sensors`: IMU, GPS, barometer, magnetometer, airspeed, camera, battery.
- `two_drones`: two-vehicle initialization, movement, kinematics.
- `wind`: sunlight, cloud shadow, wind velocity, pause/resume, sim time.
- `battery`: battery state and battery drain-rate controls.

## Scene Files

All scenario configurations are in:

`client/python/example_user_scripts/sim_config/`

Key files:

- `scene_basic_drone.jsonc`
- `scene_drone_sensors.jsonc`
- `scene_two_drones.jsonc`
- `scene_drone_wind.jsonc`
- `scene_battery_simple.jsonc`

## Success Criteria

- Exit code `0`: success.
- Exit code `1`: failure.
- Expected completion marker:

```text
[PASS] User scenario completed
```

## Troubleshooting

Connection issues:

```bash
ss -tln | grep 7777
./client/cpp/scripts/run_cpp_user_scenario.sh --simhost <sim-ip>
```

Rebuild if binary is missing:

```bash
rm -rf client/cpp/build_local
cmake -S client/cpp -B client/cpp/build_local -DCMAKE_BUILD_TYPE=Debug
cmake --build client/cpp/build_local --target cpp_client_scenarios -j$(nproc)
```
