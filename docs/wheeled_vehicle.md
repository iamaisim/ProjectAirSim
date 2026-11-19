# Native Wheeled Vehicles

Project AirSim can control an Unreal `AWheeledVehiclePawn` directly, without a
Blueprint implementing `IProjectAirSimVehicle` or a Blueprint
`SetParameterSignal` event. This integration is selected with the
`wheeled-vehicle-class` robot setting.

Use the existing Unreal Vehicle integration when a Blueprint must define its
own indexed parameter behavior. Use the native Wheeled Vehicle integration
when throttle, steering, and brake should go directly to the pawn's
`ChaosWheeledVehicleMovementComponent`.

| Integration | Robot setting | Blueprint interface required | Control mapping |
| --- | --- | --- | --- |
| Wheeled Vehicle | `wheeled-vehicle-class` | No | Fixed: throttle, steering, brake |
| Unreal Vehicle | `unreal-vehicle-class` | Yes for parameter forwarding | Defined by the Blueprint |

The two class settings are mutually exclusive in one robot configuration.

## Configuration

A minimal native wheeled-vehicle robot uses Unreal physics and points to a
Blueprint class derived from `AWheeledVehiclePawn`:

```json
{
  "physics-type": "unreal-physics",
  "wheeled-vehicle-class": "/ProjectAirSim/VehicleAdv/SUV/SuvCarPawn.SuvCarPawn_C"
}
```

The example configurations are:

- `client/python/example_user_scripts/sim_config/robot_wheeled_vehicle.jsonc`
- `client/python/example_user_scripts/sim_config/scene_wheeled_vehicle.jsonc`
- `client/python/example_user_scripts/sim_config/robot_wheeled_vehicle_simpledrive.jsonc`
- `client/python/example_user_scripts/sim_config/scene_wheeled_vehicle_simpledrive.jsonc`

The pawn must have a usable `ChaosWheeledVehicleMovementComponent`, wheel
classes, skeletal wheel bones, and drivetrain settings. When Chaos mechanical
simulation is enabled, Project AirSim applies only Chaos inputs. When it is
disabled, Project AirSim uses its direct-force fallback instead; the two
propulsion paths are never active together.

For an engine-driven Unreal scene, enable physics substepping when using a
small `step-ns`:

```json
"clock": {
  "type": "engine-driven",
  "step-ns": 3000000,
  "engine-substepping": true
}
```

This treats 3 ms as the maximum Chaos physics substep instead of requiring the
outer Unreal render loop to sustain 333 FPS.

## Vehicle control

Python and C++ use dedicated `SetThrottle`, `SetSteering`, and `SetBrakes`
RPCs with a `value` argument and boolean result. There is no `SetParameter`
alias for WheeledVehicle. ROS 2 retains its indexed `set_parameter` service:

| Index | Control | Accepted range |
| --- | --- | --- |
| `0` | Throttle | `[-1, 1]` |
| `1` | Steering | `[-1, 1]` |
| `2` | Brake | `[0, 1]` |

The bridge rejects unknown wheeled indices; the simulator rejects non-finite values and clamps valid
finite values to the ranges above.

### Python

From `client/python/example_user_scripts`:

```bash
python3 hello_wheeled_vehicle.py
```

The Python `WheeledVehicle` interface provides named control methods:

```python
vehicle.set_throttle(0.7)
vehicle.set_steering(0.45)
vehicle.set_brakes(1.0)
```

Each method sends its corresponding named RPC directly.

### C++

From the repository root:

```bash
cmake -S client/cpp -B client/cpp/build_linux/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build client/cpp/build_linux/Debug --target hello_wheeled_vehicle -j"$(nproc)"
./client/cpp/build_linux/Debug/hello_wheeled_vehicle
```

The public call is:

```cpp
bool applied = false;
vehicle.SetThrottle(0.7f, &applied);
vehicle.SetSteering(0.45f, &applied);
vehicle.SetBrakes(1.0f, &applied);
```

### ROS 2

Build and source the bridge, then configure its single-vehicle service name:

```bash
cd ros
colcon build --packages-select projectairsim_ros2_cpp
source install/setup.bash

ros2 run projectairsim_ros2_cpp projectairsim_ros2_cpp_node --ros-args \
  -p scene_config:=scene_wheeled_vehicle.jsonc \
  -p sim_config_path:=../client/python/example_user_scripts/sim_config \
  -p vehicle_name:=WheeledVehicle
```

In another sourced terminal, apply throttle, steering, and brake:

The bridge queries `/Sim/<scene>/robots/<robot>/GetRobotType` lazily and caches
valid results by scene/robot path until reconnect or a successful bridge scene
load. Robot names are arbitrary. `wheeled-vehicle` enables the mapping below;
`unreal-vehicle` forwards `SetParameter` with Blueprint-defined indices. `drone`,
`jsbsim`, and `other` reject indexed controls, leaving other services unchanged.
Missing/failed queries, malformed results and unknown types return explicit
errors without a control RPC or name-based fallback; failures are not cached.
An empty requested name uses `vehicle_name`. No JSONC index mapping is required.
The simulator must provide `GetRobotType`; its result reports the configured
backend rather than spawn or mechanical-drivetrain readiness.

To connect to an already loaded scene, omit `scene_config`; C++ World
discovers the scene and robot names from topics without fetching configuration,
reloading or resetting the scene. External scene changes require reconnecting
the bridge or loading the scene through its load service with
`is_primary_client: true`.

```bash
ros2 service call /projectairsim/WheeledVehicle/set_parameter \
  projectairsim_ros2_cpp/srv/SetParameter "{index: 0, value: 0.7}"

ros2 service call /projectairsim/WheeledVehicle/set_parameter \
  projectairsim_ros2_cpp/srv/SetParameter "{index: 1, value: 0.45}"

ros2 service call /projectairsim/WheeledVehicle/set_parameter \
  projectairsim_ros2_cpp/srv/SetParameter "{index: 2, value: 1.0}"
```

## Zero-wiring SimpleDrive

For SimpleDrive, use `hello_wheeled_vehicle_simpledrive.py`. The robot config
contains a `simple-drive-api` controller but no Unreal Vehicle actuators.
`UnrealRobot` maps the controller output directly as
`[throttle, steering, brake]` and holds those values as Chaos inputs. Client
control for this workflow continues to use the normal Rover/SimpleDrive API;
the Blueprint requires no signal wiring.

## Runtime diagnostics

The Unreal log identifies the active path:

- `WheeledVehicle zero-wiring SimpleDrive mapping active` confirms direct
  SimpleDrive output mapping.
- `Chaos mechanical drivetrain (direct-force fallback disabled)` confirms the
  pawn is using its configured Chaos drivetrain.
- `direct-force fallback active` confirms mechanical simulation is disabled.

If the pawn fails to load, verify that the asset path exists in the active
Unreal version and that its generated class derives from
`AWheeledVehiclePawn`.
