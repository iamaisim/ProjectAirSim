# Unreal Vehicle

Project AirSim can use an Unreal Engine vehicle for its dynamics while exposing
indexed parameter values to Python. The included example uses the Chaos-based SUV
from [`iamaisim/ProjectAirSim`](https://github.com/iamaisim/ProjectAirSim).

The user-facing API is `UnrealVehicle.set_parameter(index, value)`. The
`SetParameterSignal` Blueprint event receives the same index and value.

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
  "unreal-vehicle-class": "/ProjectAirSim/VehicleAdv/SUV/SuvCarPawn.SuvCarPawn_C"
}
```

The `SetParameter` service is registered for a valid Unreal vehicle actor; no
Project AirSim controller is required.

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

The script loads `scene_unreal_vehicle.jsonc`, sends indexed parameter values,
and prints kinematics while the SUV moves.

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

vehicle.set_parameter(0, 0.5)
vehicle.set_parameter(1, 0.0)
vehicle.set_parameter(2, 0.2)
```

Kinematics are available through `vehicle.get_kinematics()`. Example scripts
can define aliases for parameter indices to keep control code readable.

## How control reaches Unreal Engine

```text
Python UnrealVehicle.set_parameter(index, value)
    -> SetParameter service
    -> Project AirSim Unreal bridge
    -> Blueprint SetParameterSignal(Index, Signal)
    -> Chaos vehicle movement component
```

`SetParameter` and `SetParameterSignal` both use the parameter index and signal.

## Blueprint setup for a custom vehicle

The supplied Project AirSim SUV is ready to run without Blueprint changes. Follow this
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

![Project AirSim Vehicle listed under Implemented Interfaces](images/unreal_vehicle/implemented-interface.png)

The C++ interface is `IProjectAirSimVehicle`.

The interface exposes:

| Function | Purpose |
| --- | --- |
| `SetParameterSignal(Index, Signal)` | Receive an indexed parameter value |
| `GetPosition` / `GetRotation` | Override pose reporting when actor transform is insufficient |
| `GetLinearVelocity` / `GetAngularVelocity` | Override velocity reporting |
| `GetLinearAcceleration` / `GetAngularAcceleration` | Optional acceleration reporting |
| `ResetToSpawnPose` | Restore custom vehicle state during simulation reset |

Project AirSim can derive common kinematics from the actor and its physics
component. Override the getters only when the vehicle stores or calculates them
differently.

### Map parameters to Chaos

`SetParameterSignal` is called once per tick for each parameter received from
Python. The parameter indices are defined by the vehicle Blueprint. For the
included SUV, the current indices are:

| Index | Signal | Typical Chaos node |
| ---: | --- | --- |
| `0` | Throttle | `Set Throttle Input` |
| `1` | Brake | `Set Brake Input` |
| `2` | Steering | `Set Steering Input` |

Implement the event with a **Switch on Int**, then forward `Value` to the
corresponding function on the stored Chaos movement component.

Typical Blueprint flow:

```text
Event SetParameterSignal(Index, Signal)
    -> Switch on Int
       0 -> Set Throttle Input(Signal)
       1 -> Set Brake Input(Signal)
       2 -> Set Steering Input(Signal)
```

![SetParameterSignal mapped to Chaos vehicle inputs](images/unreal_vehicle/set-parameter-signal.png)

The mapping is an implementation detail of the Blueprint. Python sends only
indices and signals.

If the Blueprint derives from `ProjectAirSimVehicleActorBase`, the base class
stores these three signals and can apply a basic force/torque response. Set
**Apply Default Actuator Forces** to false before implementing a different
force model, otherwise both responses may act on the vehicle.

### Configure the custom Blueprint

Point `unreal-vehicle-class` at the generated Blueprint class. No controller is
needed for direct parameter forwarding:

```jsonc
{
  "physics-type": "unreal-physics",
  "unreal-vehicle-class": "/Game/Vehicles/BP_MyVehicle.BP_MyVehicle_C",
  "sensors": []
}
```

Blueprint class paths end in `_C`. Content inside the ProjectAirSim plugin uses
the `/ProjectAirSim/...` mount point; project content normally uses `/Game/...`.

Add this robot configuration to a scene and control the actor with
`UnrealVehicle`, just like the supplied SUV example.

## Optional SimpleDrive controller

Project AirSim also includes `hello_unreal_vehicle_simpledrive.py` and the corresponding
`robot_unreal_vehicle_simpledrive.jsonc` and
`scene_unreal_vehicle_simpledrive.jsonc` files. That alternative exposes the
standard `Rover` API, including `set_rover_controls` and `move_on_path_async`.
It does not replace the indexed `SetParameter` example documented above.

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

- Confirm Python uses `UnrealVehicle.set_parameter()` with valid parameter
  indices and values.
- Confirm the Blueprint implements `ProjectAirSimVehicle` and maps indices 0,
  1, and 2 to its Chaos movement component.
- Confirm the Unreal vehicle movement component is active and the wheels have
  valid Chaos configurations.

### The repository was downloaded without the SUV asset

The example SUV is distributed from the public
[`iamaisim/ProjectAirSim`](https://github.com/iamaisim/ProjectAirSim) releases
as an optional asset pack instead of being kept in the main Git repository.
Install the complete pack before opening the Unreal project.

On Windows, run from the repository root:

```powershell
.\tools\assets\install_suv_assets.ps1
```

On Linux, ensure `curl`, `unzip`, and `sha256sum` are installed, then run from
the repository root:

```bash
./tools/assets/install_suv_assets.sh
```

The installer validates the archive checksum and installs it at
`unreal/Blocks/Plugins/ProjectAirSim/Content/VehicleAdv/SUV`. It refuses to
replace an existing installation unless `-Force` (Windows) or `--force`
(Linux) is supplied.

Maintainers can create a new pack from an installed SUV directory with:

```powershell
.\tools\assets\package_suv_assets.ps1 `
    -OutputPath C:\tmp\ProjectAirSim-SUV-Assets-v1.0.0.zip
```

or on Linux:

```bash
./tools/assets/package_suv_assets.sh /tmp/ProjectAirSim-SUV-Assets-v1.0.0.zip
```

After publishing a new archive, update `tools/assets/suv-assets.json` with its
version, filename, and SHA-256 checksum.
