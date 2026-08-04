# Unreal Vehicle example

Project AirSim can use an Unreal Engine vehicle for its dynamics while exposing
named control parameters to Python. The included example uses the Chaos-based SUV
from [`iamaisim/ProjectAirSim-GM`](https://github.com/iamaisim/ProjectAirSim-GM)
with Project AirSim's `unreal-vehicle-api` controller.

The user-facing API is `UnrealVehicle.set_parameter(name, value)`. The
`SetActuatorSignal` Blueprint event is the internal endpoint used by the Unreal
bridge; it is not called directly from Python.

## Included files

The repository already contains everything needed by the example:

| Purpose | Path |
| --- | --- |
| Unreal SUV Blueprint and assets | `unreal/Blocks/Plugins/ProjectAirSim/Content/VehicleAdv/SUV/` |
| Robot configuration | `client/python/example_user_scripts/sim_config/robot_unreal_vehicle.jsonc` |
| Scene configuration | `client/python/example_user_scripts/sim_config/scene_unreal_vehicle.jsonc` |
| Runnable Python example | `client/python/example_user_scripts/hello_unreal_vehicle.py` |

The robot configuration selects the included Blueprint class:

```jsonc
{
  "physics-type": "unreal-physics",
  "unreal-vehicle-class": "/ProjectAirSim/VehicleAdv/SUV/SuvCarPawn.SuvCarPawn_C",
  "controller": {
    "id": "UnrealVehicleController",
    "type": "unreal-vehicle-api",
    "unreal-vehicle-api-settings": {
      "actuators": [
        {"name": "throttle", "default-value": 0.0},
        {"name": "brake",    "default-value": 0.0},
        {"name": "steering", "default-value": 0.0}
      ]
    }
  }
}
```

The actuator names are the public parameter names accepted by
`UnrealVehicle.set_parameter`. Their array order maps internally to throttle,
brake, and steering Blueprint signals.

## Run the example

### 1. Build and start Blocks

Build Project AirSim and launch the Blocks Unreal project using the normal
[source build instructions](development/use_source.md). Start Play in the
Unreal Editor, or launch a built Blocks executable, and leave it running while
the Python client connects.

The included SUV assets must be present under the ProjectAirSim plugin content
directory shown above. They are already included in this repository.

### 2. Install the Python client

Follow the [Python client setup](client_setup.md) and activate its environment.

### 3. Run the client script

From the repository root:

```console
cd client/python/example_user_scripts
python hello_unreal_vehicle.py
```

The script loads `scene_unreal_vehicle.jsonc`, sends named throttle, brake, and
steering parameters, and prints kinematics while the SUV moves.

The essential Python API is:

```python
from projectairsim import ProjectAirSimClient, World
from projectairsim.unreal_vehicle import UnrealVehicle

client = ProjectAirSimClient()
client.connect()

world = World(
    client,
    "scene_unreal_vehicle.jsonc",
    delay_after_load_sec=2,
)
vehicle = UnrealVehicle(client, world, "UnrealVehicle")

vehicle.set_parameter("throttle", 0.5)
vehicle.set_parameter("brake", 0.0)
vehicle.set_parameter("steering", 0.2)
```

Kinematics are available through `vehicle.get_kinematics()`. The
`set_actuator()` method remains only as a compatibility alias for
`set_parameter()` and should not be used in new examples.

## How control reaches Unreal Engine

```text
Python UnrealVehicle.set_parameter(name, value)
    -> SetParameter service
    -> unreal-vehicle-api controller
    -> ordered throttle / brake / steering signals
    -> Project AirSim Unreal bridge
    -> Blueprint SetActuatorSignal(Index, Signal)
    -> Chaos vehicle movement component
```

This distinction is important: `SetParameter` is the public named API;
`SetActuatorSignal` is the internal index-based Blueprint interface.

## Blueprint setup for a custom vehicle

The supplied GM SUV is ready to run without Blueprint changes. Follow this
section when integrating a different Chaos vehicle or an actor with custom
Unreal physics.

### Choose a Blueprint base

There are two supported approaches:

1. **Existing Chaos Pawn:** keep its current parent class and add the
   `ProjectAirSimVehicle` interface under **Class Settings > Implemented
   Interfaces**.
2. **New force-driven actor:** create a Blueprint derived from **Project AirSim
   Vehicle Actor Base**. This class implements the interface, kinematic getters,
   signal storage, and a basic force response.

For a Chaos vehicle, keeping its vehicle Pawn parent is normally the right
choice because its movement component already owns engine, wheel, suspension,
steering, and brake behavior.

### Prepare a Chaos vehicle Pawn

The Blueprint needs a configured `ChaosWheeledVehicleMovementComponent`, wheel
setups, skeletal mesh, physics asset, and collision. Verify that the vehicle can
move normally inside Unreal before connecting it to Project AirSim.

When a runtime-spawned vehicle requires explicit activation, add this setup to
**Event BeginPlay**:

1. Get the vehicle movement component.
2. Cast it to `ChaosWheeledVehicleMovementComponent`.
3. Store the result in a Blueprint variable for the control events.
4. Call **Activate** on the movement component.
5. Call **Spawn Default Controller** if the Pawn requires possession.

![BeginPlay setup that activates the Chaos vehicle movement component and spawns the default controller](images/unreal_vehicle/begin-play-setup.png)

Whether `Spawn Default Controller` is required depends on the Pawn. Do not add
it if the vehicle deliberately uses another possession or controller setup.

### Add the ProjectAirSimVehicle interface

For an existing Pawn:

1. Open the Blueprint and select **Class Settings**.
2. Find **Interfaces** in the Details panel.
3. Add **ProjectAirSimVehicle**.
4. Compile and save the Blueprint.

![Project Air Sim Vehicle listed under Implemented Interfaces](images/unreal_vehicle/implemented-interface.png)

The current C++ interface is `IProjectAirSimVehicle`. Older documentation called
it `IUnrealVehicleActor`; that name is obsolete.

The interface exposes:

| Function | Purpose |
| --- | --- |
| `SetActuatorSignal(Index, Signal)` | Receive mapped throttle, brake, and steering values |
| `GetActuatorSignal(Name)` | Optional custom signal lookup |
| `GetPosition` / `GetRotation` | Override pose reporting when actor transform is insufficient |
| `GetLinearVelocity` / `GetAngularVelocity` | Override velocity reporting |
| `GetLinearAcceleration` / `GetAngularAcceleration` | Optional acceleration reporting |
| `ResetToSpawnPose` | Restore custom vehicle state during simulation reset |

Project AirSim can derive common kinematics from the actor and its physics
component. Override the getters only when the vehicle stores or calculates them
differently.

### Map controller signals to Chaos

`SetActuatorSignal` is not called from Python. It is the final Blueprint-facing
part of the internal controller bridge. Its index order is fixed for Unreal
vehicles:

| Index | Signal | Typical Chaos node |
| ---: | --- | --- |
| `0` | Throttle | `Set Throttle Input` |
| `1` | Brake | `Set Brake Input` |
| `2` | Steering | `Set Steering Input` |

Implement the event with a **Switch on Int**, then forward `Signal` to the
corresponding function on the stored Chaos movement component.

![SetActuatorSignal Blueprint mapping actuator indices to throttle, brake, and steering](images/unreal_vehicle/set-actuator-signal.png)

Typical Blueprint flow:

```text
Event SetActuatorSignal(Index, Signal)
    -> Switch on Int
       0 -> Set Throttle Input(Signal)
       1 -> Set Brake Input(Signal)
       2 -> Set Steering Input(Signal)
```

This mapping is an implementation detail of the Blueprint. Python continues to
use named calls such as `vehicle.set_parameter("throttle", 0.5)`.

If the Blueprint derives from `ProjectAirSimVehicleActorBase`, the base class
stores these three signals and can apply a basic force/torque response. Set
**Apply Default Actuator Forces** to false before implementing a different
force model, otherwise both responses may act on the vehicle.

### Configure the custom Blueprint

Point `unreal-vehicle-class` at the generated Blueprint class and configure the
named Unreal Vehicle controller parameters:

```jsonc
{
  "physics-type": "unreal-physics",
  "unreal-vehicle-class": "/Game/Vehicles/BP_MyVehicle.BP_MyVehicle_C",
  "controller": {
    "id": "UnrealVehicleController",
    "type": "unreal-vehicle-api",
    "unreal-vehicle-api-settings": {
      "actuators": [
        {"name": "throttle", "default-value": 0.0},
        {"name": "brake",    "default-value": 0.0},
        {"name": "steering", "default-value": 0.0}
      ]
    }
  },
  "sensors": []
}
```

Blueprint class paths end in `_C`. Content inside the ProjectAirSim plugin uses
the `/ProjectAirSim/...` mount point; project content normally uses `/Game/...`.

Add this robot configuration to a scene and control the actor with
`UnrealVehicle`, just like the supplied SUV example.

## Optional SimpleDrive controller

GM also includes `hello_unreal_vehicle_simpledrive.py` and the corresponding
`robot_unreal_vehicle_simpledrive.jsonc` and
`scene_unreal_vehicle_simpledrive.jsonc` files. That alternative exposes the
standard `Rover` API, including `set_rover_controls` and `move_on_path_async`.
It does not replace the named `SetParameter` example documented above.

## Troubleshooting

### The vehicle does not spawn

- Confirm the class path is
  `/ProjectAirSim/VehicleAdv/SUV/SuvCarPawn.SuvCarPawn_C`.
- Confirm the SUV assets are present in the ProjectAirSim plugin.
- Check `projectairsim_server.log` for class-loading errors.

### The script cannot find the scene configuration

Run it from `client/python/example_user_scripts`, as shown above, so the default
simulation configuration directory resolves to its `sim_config` subdirectory.

### The vehicle spawns but does not move

- Confirm Python uses `UnrealVehicle.set_parameter()` with `throttle`, `brake`,
  and `steering`.
- Confirm the robot controller type is `unreal-vehicle-api`.
- Confirm the Blueprint implements `ProjectAirSimVehicle` and maps indices 0,
  1, and 2 to its Chaos movement component.
- Confirm the Unreal vehicle movement component is active and the wheels have
  valid Chaos configurations.

### The repository was downloaded without the SUV asset

The Blueprint depends on the complete `VehicleAdv/SUV` content directory. A Git
LFS pointer or a partial asset download is not sufficient; obtain the full
example content before opening the Unreal project.
